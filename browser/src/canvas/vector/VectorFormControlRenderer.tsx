/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace cool {
  /*
		Draws a form control in Online's look, from the values of its model. The HTML element of
		its kind is built with the classes of JSDialog's widgets, so the page's own rules style
		it; its computed look is written into it, the images it uses become data, and the whole
		goes into an SVG image as a foreignObject, which is drawn into the control's unit square.
		The image is made once for each primitive and kept with it, until the server sends the
		object anew. It loads on its own, so the control shows from the draw after that on.
	*/
  export class VectorFormControlRenderer {
    /// How many twips one CSS pixel is at a zoom of one.
    private static readonly twipsPerPixel = 15;

    /// The look of an element that goes into the picture, read from the page's rules.
    private static readonly lookProperties = [
      'display',
      'flex-direction',
      'align-items',
      'justify-content',
      'box-sizing',
      'margin',
      'padding',
      'border-top',
      'border-right',
      'border-bottom',
      'border-left',
      'border-radius',
      'background-color',
      'background-image',
      'background-repeat',
      'background-position',
      'background-size',
      'color',
      'font-family',
      'font-size',
      'font-weight',
      'font-style',
      'line-height',
      'text-align',
      'white-space',
      'vertical-align',
      'width',
      'height',
      'opacity',
      'appearance',
      'overflow',
    ];

    /// What a font the model names falls back to where the browser has not got it.
    private static readonly _fallbackFont = 'sans-serif';

    private _pictures = new WeakMap<FormControlPrimitive, HTMLImageElement>();
    private _onPictureReady: (() => void) | undefined;

    constructor(onPictureReady?: () => void) {
      this._onPictureReady = onPictureReady;
    }

    render(
      context: CanvasRenderingContext2D,
      primitive: FormControlPrimitive,
    ): void {
      const matrix = Matrix2D.fromArray(primitive.matrix);
      if (!matrix) return;

      const picture = this._pictures.get(primitive);
      if (!picture) {
        const made = new Image();
        this._pictures.set(primitive, made);
        void this._paint(primitive, matrix, made);
        return;
      }
      if (!picture.complete || !picture.naturalWidth) return;

      context.save();
      matrix.applyTo(context);
      context.drawImage(picture, 0, 0, 1, 1);
      context.restore();
    }

    // the element of the control's kind, with the classes of JSDialog's widgets
    private _build(primitive: FormControlPrimitive): HTMLElement {
      const label = primitive.label ?? '';
      const checked = primitive.state === 1;
      switch (primitive.kind) {
        case 'checkBox':
          return (
            <div class="checkbutton jsdialog">
              <input type="checkbox" checked={checked} />
              <label>{label}</label>
            </div>
          );
        case 'optionButton':
          return (
            <div class="radiobutton jsdialog">
              <input type="radio" checked={checked} />
              <label>{label}</label>
            </div>
          );
        default:
          return (
            <button class="ui-pushbutton jsdialog">
              <span>{label}</span>
            </button>
          );
      }
    }

    // what the model sets itself goes over the look of the widget
    private _applyModel(
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const style = element.style;
      style.boxSizing = 'border-box';
      if (primitive.textColor) style.color = primitive.textColor;
      if (primitive.background) style.backgroundColor = primitive.background;
      const align =
        primitive.align !== undefined
          ? (['left', 'center', 'right'][primitive.align] ?? '')
          : primitive.kind === 'pushButton'
            ? 'center'
            : 'left';
      // the box of a check box or an option button stands at the start, its label next to it,
      // and the alignment is the label's
      if (primitive.kind === 'pushButton') style.textAlign = align;
      else {
        style.justifyContent = 'flex-start';
        element.querySelectorAll('label').forEach((label: HTMLElement) => {
          label.style.flex = '1';
          label.style.textAlign = align;
          // a small gap after the box, as in the office
          label.style.padding = '0 0 0 4px';
        });
      }
      const font = primitive.font;
      // an SVG image gets no system-ui, so the family ends in a generic one it has
      style.fontFamily =
        (font?.name ? '"' + font.name + '", ' : '') +
        VectorFormControlRenderer._fallbackFont;
      if (font?.height) style.fontSize = font.height + 'pt';
      if (font?.bold) style.fontWeight = 'bold';
      if (font?.italic) style.fontStyle = 'italic';
      if (primitive.disabled)
        element
          .querySelectorAll('button, input')
          .forEach((input: Element) => input.setAttribute('disabled', ''));
      // the label of a check box or an option button takes the model's font too
      element.querySelectorAll('label').forEach((label: HTMLElement) => {
        label.style.font = 'inherit';
        label.style.color = 'inherit';
      });
    }

    private async _paint(
      primitive: FormControlPrimitive,
      matrix: Matrix2D,
      picture: HTMLImageElement,
    ): Promise<void> {
      const origin = matrix.apply(0, 0);
      const across = matrix.apply(1, 0);
      const down = matrix.apply(0, 1);
      const width = Math.max(
        1,
        Math.round(
          Math.hypot(across.x - origin.x, across.y - origin.y) /
            VectorFormControlRenderer.twipsPerPixel,
        ),
      );
      const height = Math.max(
        1,
        Math.round(
          Math.hypot(down.x - origin.x, down.y - origin.y) /
            VectorFormControlRenderer.twipsPerPixel,
        ),
      );

      // built out of view in the page, so the page's rules give it its look
      const control = this._build(primitive);
      this._applyModel(control, primitive);
      control.style.width = width + 'px';
      control.style.height = height + 'px';
      const holder = (
        <div
          style={
            'position: fixed; left: -10000px; top: 0; font-family: var(--cool-font);' +
            ' width: ' +
            width +
            'px; height: ' +
            height +
            'px;'
          }
        >
          {control}
        </div>
      );
      document.body.appendChild(holder);
      let markup: string;
      try {
        await this._writeLookInto(control);
        markup = new XMLSerializer().serializeToString(control);
      } finally {
        holder.remove();
      }

      const svg =
        '<svg xmlns="http://www.w3.org/2000/svg" width="' +
        width +
        '" height="' +
        height +
        '"><foreignObject width="100%" height="100%">' +
        '<div xmlns="http://www.w3.org/1999/xhtml" style="width: 100%; height: 100%;">' +
        markup +
        '</div></foreignObject></svg>';

      picture.onload = () => this._onPictureReady?.();
      picture.src =
        'data:image/svg+xml;charset=utf-8,' + encodeURIComponent(svg);
    }

    // writes the computed look of the element and all it holds into their style, with the
    // images it uses as data, as an SVG image takes nothing from the page
    private async _writeLookInto(element: Element): Promise<void> {
      const elements = [element, ...Array.from(element.querySelectorAll('*'))];
      const looks = elements.map((one: Element) => {
        const computed = window.getComputedStyle(one);
        return VectorFormControlRenderer.lookProperties.map(
          (name: string): [string, string] => [
            name,
            computed.getPropertyValue(name),
          ],
        );
      });

      await Promise.all(
        looks.flat().map(async (entry: [string, string]) => {
          const found = /url\("?([^")]+)"?\)/.exec(entry[1]);
          if (!found || found[1].startsWith('data:')) return;
          const data = await VectorFormControlRenderer._asData(found[1]);
          if (data)
            entry[1] = entry[1].replace(found[0], 'url("' + data + '")');
        }),
      );

      elements.forEach((one: Element, at: number) => {
        const style = (one as HTMLElement).style;
        for (const [name, value] of looks[at])
          if (value) style.setProperty(name, value);
      });
      // the edge of the picture is the control's frame, so the control itself has no margin
      (element as HTMLElement).style.margin = '0';
    }

    private static async _asData(url: string): Promise<string | undefined> {
      try {
        const response = await fetch(url);
        if (!response.ok) return undefined;
        const blob = await response.blob();
        return await new Promise<string | undefined>((resolve) => {
          const reader = new FileReader();
          reader.onload = () => resolve(reader.result as string);
          reader.onerror = () => resolve(undefined);
          reader.readAsDataURL(blob);
        });
      } catch {
        return undefined;
      }
    }
  }
}
