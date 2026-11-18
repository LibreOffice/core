/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
  /// The thumbnail of one slide: its objects in paint order with the master drawn in, and the
  /// images and fonts they draw with, each font cut down and under an id of its own.
  export interface VectorThumbnailResponse {
    type: string;
    part?: number;
    dpi?: number;
    objects?: SlideObject[];
    graphics?: VectorRenderingGraphicsResponse[];
    fonts?: VectorRenderingFontResponse[];
  }
}

/// Draws a vector thumbnail with the images and fonts it carries, apart from the vector content of
/// the document on screen.
class VectorThumbnail {
  /// Whether the font with an id loaded, for every font a thumbnail carried. The id is the hash of
  /// the font file, so one load serves every thumbnail that names it.
  private static readonly _fonts = new Map<string, Promise<boolean>>();

  /// The thumbnail drawn on a canvas of the given size in device pixels, once its images decoded
  /// and its fonts loaded. Undefined when the thumbnail holds no page.
  static async draw(
    thumbnail: cool.VectorThumbnailResponse,
    width: number,
    height: number,
  ): Promise<HTMLCanvasElement | undefined> {
    const objects = thumbnail.objects ?? [];
    const [pageWidth, pageHeight] = VectorPartManagerBase.pageBoundsOf(objects);
    if (pageWidth <= 0 || pageHeight <= 0 || width <= 0 || height <= 0)
      return undefined;

    const images = new Map<number, HTMLImageElement>();
    const fonts = new Set<string>();
    await Promise.all([
      ...(thumbnail.graphics ?? []).map((graphic) =>
        VectorThumbnail._decodeImage(graphic, images),
      ),
      ...(thumbnail.fonts ?? []).map(async (font) => {
        if (await VectorThumbnail._loadFont(font)) fonts.add(font.fontId);
      }),
    ]);

    const canvas = (
      <canvas width={width} height={height} />
    ) as HTMLCanvasElement;
    const context = canvas.getContext('2d');
    if (!context) return undefined;

    // Twips to canvas pixels.
    context.scale(width / pageWidth, height / pageHeight);

    const renderer = new cool.VectorPrimitiveRenderer(
      (checksum) => images.get(checksum),
      (fontId) => fonts.has(fontId),
    );
    renderer.setSlideBounds(pageWidth, pageHeight);
    renderer.setUpPageColors(VectorPartManagerBase.pageEntryOf(objects));
    for (const object of objects) {
      for (const primitive of object.primitives ?? [])
        renderer.renderPrimitive(context, primitive);
    }
    renderer.releaseScratchCanvases();
    return canvas;
  }

  /// Decode the image of a graphic into images, by its checksum. An image that does not decode
  /// stays out, and the primitives that name it draw nothing.
  private static async _decodeImage(
    graphic: cool.VectorRenderingGraphicsResponse,
    images: Map<number, HTMLImageElement>,
  ): Promise<void> {
    if (!graphic.data) return;
    const image = new Image();
    image.src = graphic.data;
    try {
      await image.decode();
      images.set(graphic.checksum, image);
    } catch (error) {
      window.app.console.warn(
        'vectorthumbnail: image ' + graphic.checksum + ' does not decode',
      );
    }
  }

  /// Load a font a thumbnail carries, once per id. False when it does not load, and the text then
  /// draws in its family name.
  private static _loadFont(
    font: cool.VectorRenderingFontResponse,
  ): Promise<boolean> {
    let loaded = VectorThumbnail._fonts.get(font.fontId);
    if (!loaded) {
      loaded = font.data
        ? VectorResourceLoader.loadFontFace(font.fontId, font.data).then(
            () => true,
            () => false,
          )
        : Promise.resolve(false);
      VectorThumbnail._fonts.set(font.fontId, loaded);
    }
    return loaded;
  }
}
