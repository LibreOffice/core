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

    // the arrows the small buttons show
    private static readonly arrowUp = '\u25B4';
    private static readonly arrowDown = '\u25BE';
    private static readonly arrowLeft = '\u25C2';
    private static readonly arrowRight = '\u25B8';

    // a box with an arrow in it, as a drop-down, a spin, a scroll and a navigation button show it
    private static _arrowBox(
      arrow: string,
      grow: boolean = false,
    ): HTMLElement {
      return (
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            flex: grow ? '1 1 0' : '0 0 auto',
            minWidth: '1.2em',
            minHeight: '0',
            boxSizing: 'border-box',
            padding: '0 2px',
            border: '1px solid var(--color-border-input-dark)',
            backgroundColor: 'var(--color-background-dark)',
            color: 'var(--color-text-dark)',
            fontSize: '0.8em',
            lineHeight: '1',
          }}
        >
          {arrow}
        </div>
      );
    }

    // a field with its text, with a drop-down button or spin buttons at its end where it has them
    private static _field(
      primitive: FormControlPrimitive,
      text: string,
    ): HTMLElement {
      const field = (
        <div
          class="jsdialog ui-edit"
          style={{
            display: 'flex',
            alignItems: primitive.multiLine ? 'flex-start' : 'center',
            alignSelf: 'stretch',
            flex: '1',
            maxHeight: 'none',
            padding: '0',
            overflow: 'hidden',
          }}
        >
          <div
            style={{
              flex: '1',
              padding: '0 4px',
              overflow: 'hidden',
              whiteSpace: primitive.multiLine ? 'pre-wrap' : 'nowrap',
              lineHeight: 'normal',
            }}
          >
            {text}
          </div>
        </div>
      );
      const renderer = VectorFormControlRenderer;
      if (primitive.dropdown)
        field.appendChild(renderer._arrowBox(renderer.arrowDown));
      if (primitive.spin)
        field.appendChild(
          <div
            style={{
              display: 'flex',
              flexDirection: 'column',
              alignSelf: 'stretch',
            }}
          >
            {renderer._arrowBox(renderer.arrowUp, true)}
            {renderer._arrowBox(renderer.arrowDown, true)}
          </div>,
        );
      return field;
    }

    // the entries of a list, the selected ones in the selection's colour
    private static _list(primitive: FormControlPrimitive): HTMLElement {
      const selected = new Set(primitive.selected ?? []);
      return (
        <div
          class="jsdialog ui-listbox"
          style={{
            display: 'block',
            height: '100%',
            padding: '0',
            overflow: 'hidden',
          }}
        >
          {(primitive.items ?? []).map((item: string, at: number) => (
            <div
              style={
                selected.has(at)
                  ? {
                      padding: '0 4px',
                      backgroundColor: 'var(--color-primary)',
                      color: 'var(--color-primary-text)',
                    }
                  : { padding: '0 4px' }
              }
            >
              {item}
            </div>
          ))}
        </div>
      );
    }

    // a scroll bar: its two arrows and the thumb on the track between, where the value stands
    private static _scrollBar(primitive: FormControlPrimitive): HTMLElement {
      const renderer = VectorFormControlRenderer;
      const vertical = primitive.vertical === true;
      const min = primitive.min ?? 0;
      const max = primitive.max ?? 100;
      const range = Math.max(1, max - min);
      const visible = Math.min(
        range,
        Math.max(1, primitive.visible ?? range / 10),
      );
      const at = Math.min(
        1,
        Math.max(0, ((primitive.value ?? min) - min) / range),
      );
      const thumbShare = (visible / (range + visible)) * 100;
      const thumbStart = at * (100 - thumbShare);
      const thumb = (
        <div
          style={{
            position: 'absolute',
            backgroundColor: 'var(--color-border-input-dark)',
            borderRadius: '2px',
            ...(vertical
              ? {
                  left: '2px',
                  right: '2px',
                  top: thumbStart + '%',
                  height: thumbShare + '%',
                }
              : {
                  top: '2px',
                  bottom: '2px',
                  left: thumbStart + '%',
                  width: thumbShare + '%',
                }),
          }}
        />
      );
      return (
        <div
          class="jsdialog"
          style={{
            display: 'flex',
            flexDirection: vertical ? 'column' : 'row',
          }}
        >
          {renderer._arrowBox(vertical ? renderer.arrowUp : renderer.arrowLeft)}
          <div
            style={{
              position: 'relative',
              flex: '1',
              backgroundColor: 'var(--color-background-dark)',
            }}
          >
            {thumb}
          </div>
          {renderer._arrowBox(
            vertical ? renderer.arrowDown : renderer.arrowRight,
          )}
        </div>
      );
    }

    // a navigation bar: where the record stands and the buttons that go through the records,
    // greyed as the office shows it while the form has no data source to go through
    private static _navigationBar(compact: boolean = false): HTMLElement {
      const renderer = VectorFormControlRenderer;
      const small = (
        <div
          class="jsdialog ui-edit"
          style={{
            width: '3em',
            height: '80%',
            padding: '0',
            maxHeight: 'none',
          }}
        />
      );
      return (
        <div
          class="jsdialog"
          style={{
            display: 'flex',
            alignItems: 'center',
            gap: '4px',
            padding: '0 4px',
            backgroundColor: 'var(--color-background-lighter)',
            opacity: '0.5',
            ...(compact
              ? { flex: '0 0 auto', height: '1.5em', fontSize: '0.8em' }
              : {}),
          }}
        >
          <span>{_('Record')}</span>
          {small}
          <span>{_('of')}</span>
          {small.cloneNode(true)}
          {renderer._arrowBox('|' + renderer.arrowLeft)}
          {renderer._arrowBox(renderer.arrowLeft)}
          {renderer._arrowBox(renderer.arrowRight)}
          {renderer._arrowBox(renderer.arrowRight + '|')}
          {renderer._arrowBox(renderer.arrowRight + '*')}
        </div>
      );
    }

    // a table control: the labels of its columns over an empty row, a marker column in front
    private static _table(primitive: FormControlPrimitive): HTMLElement {
      const cell = {
        border: '1px solid var(--color-border-input-dark)',
        padding: '0 4px',
        textAlign: 'left',
        fontWeight: 'normal',
        height: '1.2em',
      };
      const columns = primitive.columns?.length ? primitive.columns : [''];
      return (
        <div
          class="jsdialog"
          style={{
            display: 'flex',
            flexDirection: 'column',
            backgroundColor: 'var(--color-background-lighter)',
            border: '1px solid var(--color-border-input-dark)',
          }}
        >
          <table
            style={{ borderCollapse: 'collapse', width: '100%', flex: '1' }}
          >
            <tr style={{ backgroundColor: 'var(--color-background-dark)' }}>
              <th style={{ ...cell, width: '1.5em' }} />
              {columns.map((label: string) => (
                <th style={cell}>{label}</th>
              ))}
            </tr>
            <tr style={{ height: '100%' }}>
              <td
                style={{
                  ...cell,
                  backgroundColor: 'var(--color-background-dark)',
                }}
              >
                {VectorFormControlRenderer.arrowRight}
              </td>
              {columns.map(() => (
                <td
                  style={{
                    ...cell,
                    backgroundColor: 'var(--color-background-lighter)',
                  }}
                />
              ))}
            </tr>
          </table>
          {VectorFormControlRenderer._navigationBar(true)}
        </div>
      );
    }

    // the element of the control's kind, with the classes of JSDialog's widgets
    private _build(primitive: FormControlPrimitive): HTMLElement {
      const renderer = VectorFormControlRenderer;
      const label = primitive.label ?? '';
      const text = primitive.text ?? '';
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
        case 'label':
          return (
            <div
              class="jsdialog ui-text"
              style={{ display: 'block', backgroundColor: 'transparent' }}
            >
              {label}
            </div>
          );
        case 'groupBox':
          return (
            <fieldset
              class="jsdialog"
              style={{
                margin: '0',
                padding: '0 4px',
                border: '1px solid var(--color-border-input-dark)',
                borderRadius: 'var(--border-radius)',
              }}
            >
              <legend style={{ padding: '0 2px' }}>{label}</legend>
            </fieldset>
          );
        case 'textBox':
        case 'formattedField':
        case 'numericField':
        case 'currencyField':
        case 'dateField':
        case 'timeField':
        case 'patternField':
        case 'comboBox':
          return renderer._field(primitive, text);
        case 'fileSelection':
          return (
            <div
              class="jsdialog"
              style={{ display: 'flex', alignItems: 'stretch', gap: '0' }}
            >
              {renderer._field(primitive, text)}
              <button class="ui-pushbutton jsdialog">
                <span>{_('Browse...')}</span>
              </button>
            </div>
          );
        case 'listBox':
          return primitive.dropdown
            ? renderer._field(
                primitive,
                (primitive.items ?? [])[(primitive.selected ?? [])[0]] ?? '',
              )
            : renderer._list(primitive);
        case 'imageButton':
        case 'imageControl':
          return (
            <div
              class="jsdialog"
              style={{ border: '1px solid var(--color-border-input-dark)' }}
            />
          );
        case 'scrollBar':
          return renderer._scrollBar(primitive);
        case 'spinButton':
          return (
            <div
              class="jsdialog"
              style={{
                display: 'flex',
                flexDirection: primitive.vertical ? 'column' : 'row',
              }}
            >
              {renderer._arrowBox(
                primitive.vertical ? renderer.arrowUp : renderer.arrowLeft,
                true,
              )}
              {renderer._arrowBox(
                primitive.vertical ? renderer.arrowDown : renderer.arrowRight,
                true,
              )}
            </div>
          );
        case 'tableControl':
          return renderer._table(primitive);
        case 'navigationBar':
          return renderer._navigationBar();
        default:
          // a push button
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
        // the control itself may be the button, as a push button is
        [element, ...Array.from(element.querySelectorAll('button, input'))]
          .filter((input: Element) => input.matches('button, input'))
          .forEach((input: Element) => input.setAttribute('disabled', ''));
      // all text in the control takes the model's font, a label and a set text colour its colour
      element
        .querySelectorAll<HTMLElement>('*')
        .forEach((inner: HTMLElement) => {
          inner.style.font = 'inherit';
          if (primitive.textColor || inner.tagName === 'LABEL')
            inner.style.color = 'inherit';
        });
      // a control without a border loses the border of its field too
      if (primitive.border === 0) {
        style.border = 'none';
        element
          .querySelectorAll<HTMLElement>('.ui-edit')
          .forEach((field: HTMLElement) => {
            field.style.border = 'none';
          });
      }
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
          style={{
            position: 'fixed',
            left: '-10000px',
            top: '0',
            fontFamily: 'var(--cool-font)',
            width: width + 'px',
            height: height + 'px',
          }}
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
