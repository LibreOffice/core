/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
	The player of a selected video or audio object, where the client draws the selection itself.
	It is a DOM element laid over the object's box, since the page's canvas cannot play anything,
	and it follows the box wherever the object is taken. While the document is being edited it
	starts half see-through and lets presses through to the selection underneath, so the object
	can still be dragged; a click on the object wakes it, and from then on its controls take the
	presses. Outside of editing it is awake from the start.
*/
class MediaPlayerSection extends HTMLObjectSection {
  public static readonly sectionName: string = 'media-player';

  /// The address the player plays from, so a repeated selection of the same object keeps it.
  public readonly url: string;

  constructor(
    description: { url: string; mimeType: string; srt?: string },
    corner: number[],
    size: number[],
  ) {
    super(
      MediaPlayerSection.sectionName,
      Math.round(size[0] / app.dpiScale),
      Math.round(size[1] / app.dpiScale),
      cool.SimplePoint.fromCorePixels([corner[0], corner[1]]),
      'media-player',
      true,
    );

    this.url = description.url;

    // The section is only the player's place on the canvas. The presses on that place belong
    // to the selection underneath until the player is awake, and then to its controls, which
    // take them in the DOM before the canvas sees them.
    this.interactable = false;

    const player = (
      <video controls="controls" style="width: 100%; height: 100%">
        <source src={description.url} type={description.mimeType} />
        {description.srt ? (
          <track src={description.srt} kind="subtitles" />
        ) : null}
      </video>
    ) as HTMLVideoElement;

    const host = this.getHTMLObject();
    host.appendChild(player);
    // Above the handles' canvas, or the controls could not be reached.
    host.style.zIndex = '11';

    ShapeHandlesSection.addVideoSupportHandlers([player]);

    if (app.map.isEditMode()) host.style.opacity = '0.5';
    else this.wake();
  }

  /// Shows the player whole and gives it the presses, so its controls can be used.
  public wake(): void {
    const host = this.getHTMLObject();
    host.style.opacity = '1';
    host.style.pointerEvents = '';
  }

  /// Lays the player on the box of the selection as it stands now.
  public follow(corner: number[], size: number[]): void {
    const host = this.getHTMLObject();
    const width = Math.round(size[0] / app.dpiScale);
    const height = Math.round(size[1] / app.dpiScale);

    this.size = [width * app.dpiScale, height * app.dpiScale];
    host.style.width = width + 'px';
    host.style.height = height + 'px';

    this.setPosition(corner[0], corner[1]);
    this.adjustHTMLObjectPosition();
  }
}

app.definitions.mediaPlayerSection = MediaPlayerSection;
