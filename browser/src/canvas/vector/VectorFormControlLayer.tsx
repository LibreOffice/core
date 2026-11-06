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

/* global app RenderManager */

namespace cool {
  /*
		The form controls of the page the view edits, as working HTML elements over the canvas,
		while the form controls of the view are not in design mode. Each element lies where its
		control lies on the page and follows the scroll and the zoom. An element changes nothing
		itself: what is done with it goes to the server, which operates its own control there, so
		the events and macros of the form run as in the office, and the changed control comes
		back with its object.
	*/
  export class VectorFormControlLayer {
    // the objectKind of a form control
    private static readonly formControlKind: number = 32;

    /// How many twips one CSS pixel is at a zoom of one.
    private static readonly twipsPerPixel = 15;

    private static _holder: HTMLElement | undefined;

    // the element of each control by its object id, with what it was built from
    private static _elements = new Map<
      number,
      { built: string; element: HTMLElement }
    >();

    /// The id of the page the elements are shown for, empty where none are shown.
    public static shownPart: string = '';

    /// Places the elements of the form controls of the page, or takes them all away where
    /// data is absent. originX and originY are where the page's origin lies on the canvas
    /// and scale the canvas pixels to a twip.
    public static update(
      data: VectorPrimitivesData | undefined,
      originX: number,
      originY: number,
      scale: number,
    ): void {
      const layer = VectorFormControlLayer;
      const shown = new Set<number>();
      layer.shownPart = data ? String(data.partId) : '';
      if (data && !app.map.isFormControlDesignMode()) {
        for (const id of data.order) {
          const object = data.objects.get(id);
          if (object?.objectKind !== layer.formControlKind) continue;
          if (
            object.layer !== undefined &&
            !RenderManager.isLayerVisible(object.layer)
          )
            continue;
          const primitive = layer._primitiveIn(object.primitives ?? []);
          if (!primitive) continue;
          shown.add(id);
          layer._place(id, primitive, originX, originY, scale);
        }
      }

      for (const [id, entry] of layer._elements)
        if (!shown.has(id)) {
          entry.element.remove();
          layer._elements.delete(id);
        }
    }

    // the form control among the primitives of an object, at any depth
    private static _primitiveIn(
      primitives: Primitive[],
    ): FormControlPrimitive | undefined {
      for (const primitive of primitives) {
        if (primitive.type === FormControlPrimitive.type)
          return primitive as FormControlPrimitive;
        const children = (primitive as { children?: Primitive[] }).children;
        const found = children
          ? VectorFormControlLayer._primitiveIn(children)
          : undefined;
        if (found) return found;
      }
      return undefined;
    }

    private static _place(
      id: number,
      primitive: FormControlPrimitive,
      originX: number,
      originY: number,
      scale: number,
    ): void {
      const layer = VectorFormControlLayer;
      const matrix = Matrix2D.fromArray(primitive.matrix);
      if (!matrix) return;

      // a changed control is made anew; the page sends its objects again unchanged as well
      const built = JSON.stringify(primitive);
      let entry = layer._elements.get(id);
      if (!entry || entry.built !== built) {
        entry?.element.remove();
        entry = { built: built, element: layer._make(id, primitive) };
        layer._elements.set(id, entry);
        layer._holderElement().appendChild(entry.element);
      }

      // the element has its size at a zoom of one and is scaled to the view's zoom
      const origin = matrix.apply(0, 0);
      const style = entry.element.style;
      style.left = (originX + origin.x * scale) / app.dpiScale + 'px';
      style.top = (originY + origin.y * scale) / app.dpiScale + 'px';
      style.transform =
        'scale(' + (scale * layer.twipsPerPixel) / app.dpiScale + ')';
    }

    // the element over the canvas that holds the controls; it lets the pointer through
    // where no control is
    private static _holderElement(): HTMLElement {
      const layer = VectorFormControlLayer;
      if (!layer._holder || !layer._holder.isConnected) {
        layer._holder = (
          <div
            class="vector-form-controls"
            style={{
              position: 'absolute',
              left: '0',
              top: '0',
              right: '0',
              bottom: '0',
              overflow: 'hidden',
              pointerEvents: 'none',
            }}
          />
        );
        document.getElementById('canvas-container')?.appendChild(layer._holder);

        // a drag that starts on the page goes on over the controls, a selection frame or a
        // move of an object
        const holder = layer._holder;
        window.addEventListener(
          'pointerdown',
          (event: PointerEvent) => {
            if (!holder.contains(event.target as Node))
              holder.classList.add('passing');
          },
          true,
        );
        window.addEventListener(
          'pointerup',
          () => holder.classList.remove('passing'),
          true,
        );
      }
      return layer._holder;
    }

    private static _make(
      id: number,
      primitive: FormControlPrimitive,
    ): HTMLElement {
      const layer = VectorFormControlLayer;
      const control = VectorFormControlRenderer.element(primitive);
      // the element is laid exactly over the control's square
      control.style.margin = '0';
      const element = (
        <div
          class="vector-form-control"
          style={{
            position: 'absolute',
            transformOrigin: '0 0',
            pointerEvents: 'auto',
            fontFamily: 'var(--cool-font)',
          }}
        >
          {control}
        </div>
      );

      switch (primitive.kind) {
        case 'pushButton':
        case 'imageButton':
          element.addEventListener('click', () => layer._send(id, 'press', ''));
          break;
        case 'checkBox':
          element
            .querySelector('input')
            ?.addEventListener('change', (event: Event) =>
              layer._send(
                id,
                'state',
                (event.target as HTMLInputElement).checked ? '1' : '0',
              ),
            );
          break;
        case 'optionButton':
          element
            .querySelector('input')
            ?.addEventListener('change', () => layer._send(id, 'check', ''));
          break;
        case 'textBox':
        case 'formattedField':
        case 'numericField':
        case 'currencyField':
        case 'dateField':
        case 'timeField':
        case 'patternField':
        case 'comboBox':
          layer._makeTypable(id, element, primitive);
          break;
        case 'listBox':
          if (primitive.dropdown)
            layer._makeDropDownList(id, element, primitive);
          else layer._makeList(id, element, primitive);
          break;
        case 'fileSelection':
          layer._makeTypable(id, element, primitive);
          // the kit has no file picker for it, the path is typed
          element.querySelector('button')?.setAttribute('disabled', '');
          break;
        case 'spinButton':
          layer._makeSpinButton(id, element, primitive);
          break;
        case 'scrollBar':
          layer._makeScrollBar(id, element, primitive);
          break;
      }

      // the keyboard stays with the document: the controls take no focus, by click or by Tab
      element
        .querySelectorAll<HTMLElement>('button, input, select, textarea, a')
        .forEach((focusable: HTMLElement) =>
          focusable.setAttribute('tabindex', '-1'),
        );
      // a field takes the focus, as it is typed into
      element.addEventListener('mousedown', (event: MouseEvent) => {
        if (!(event.target as Element).classList.contains('vector-form-field'))
          event.preventDefault();
      });

      // the wheel scrolls and zooms the document, as it does over the page
      element.addEventListener(
        'wheel',
        (event: WheelEvent) => {
          event.preventDefault();
          document
            .getElementById('document-canvas')
            ?.dispatchEvent(new WheelEvent('wheel', event));
        },
        { passive: false },
      );

      // the label of a check box or an option button takes the click as the box does
      const input = element.querySelector('input');
      const label = element.querySelector('label');
      if (input && label)
        label.addEventListener('click', () => {
          if (!input.disabled) input.click();
        });

      return element;
    }

    /*
      The text a field shows becomes an input to type in, a text area where the field holds
      several lines. Return, or leaving the field, sends what was typed; Escape takes it back.
      The keys typed there are the field's, the document does not see them, and Return and
      Escape give the keyboard back to the document.
    */
    private static _makeTypable(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const shown = element.querySelector('.ui-edit')?.firstElementChild;
      if (!shown) return;

      const original = primitive.text ?? '';
      const style = {
        flex: '1',
        minWidth: '0',
        alignSelf: 'stretch',
        boxSizing: 'border-box',
        margin: '0',
        padding: '0 4px',
        border: 'none',
        backgroundColor: 'transparent',
        font: 'inherit',
        color: 'inherit',
        textAlign:
          primitive.align !== undefined
            ? (['left', 'center', 'right'][primitive.align] ?? 'left')
            : 'left',
        resize: 'none',
      };
      const input = (
        primitive.multiLine ? (
          <textarea class="vector-form-field" style={style} />
        ) : (
          <input type="text" class="vector-form-field" style={style} />
        )
      ) as HTMLInputElement | HTMLTextAreaElement;
      input.value = original;
      if (primitive.disabled) input.disabled = true;
      shown.replaceWith(input);

      input.addEventListener('change', () =>
        VectorFormControlLayer._send(id, 'text', input.value),
      );
      input.addEventListener('keydown', (event: Event) => {
        event.stopPropagation();
        const key = (event as KeyboardEvent).key;
        if (key === 'Escape') input.value = original;
        if (key === 'Escape' || (key === 'Enter' && !primitive.multiLine)) {
          event.preventDefault();
          input.blur();
          app.map.focus();
        }
      });
      for (const type of ['keypress', 'keyup', 'input'])
        input.addEventListener(type, (event: Event) => event.stopPropagation());

      const layer = VectorFormControlLayer;
      const extras = Array.from(input.parentElement?.children ?? []).slice(1);
      const dropArrow = primitive.dropdown
        ? (extras[0] as HTMLElement | undefined)
        : undefined;
      const spinArrows = primitive.spin
        ? extras[primitive.dropdown ? 1 : 0]?.children
        : undefined;

      // the spin buttons step the value as the field's format says; what was typed goes first
      const spin = (up: boolean) => {
        if (input.disabled) return;
        if (input.value !== original) layer._send(id, 'text', input.value);
        layer._send(id, 'spin', up ? 'up' : 'down');
      };
      if (spinArrows && spinArrows.length >= 2) {
        (spinArrows[0] as HTMLElement).addEventListener('click', () =>
          spin(true),
        );
        (spinArrows[1] as HTMLElement).addEventListener('click', () =>
          spin(false),
        );
        input.addEventListener('keydown', (event: Event) => {
          const key = (event as KeyboardEvent).key;
          if (key !== 'ArrowUp' && key !== 'ArrowDown') return;
          event.preventDefault();
          spin(key === 'ArrowUp');
        });
      }
      if (!dropArrow) return;
      dropArrow.style.cursor = 'pointer';

      // a combo box offers its entries in a list under the field, which its arrow opens
      if (primitive.kind === 'comboBox' && primitive.items?.length) {
        const entries = primitive.items;
        dropArrow.addEventListener('click', () => {
          if (input.disabled || layer._closeOpenPart(element)) return;
          const close = layer._openUnder(
            element,
            <div
              class="jsdialog ui-listbox"
              style={{
                height: 'auto',
                maxHeight: 'none',
                display: 'block',
                padding: '0',
              }}
            >
              {entries.map((entry: string) => (
                <div
                  style={{ padding: '0 4px', cursor: 'pointer' }}
                  onClick={() => {
                    input.value = entry;
                    close();
                    layer._send(id, 'text', entry);
                    app.map.focus();
                  }}
                >
                  {entry}
                </div>
              ))}
            </div>,
          );
        });
      }

      // a date field opens a calendar of the month of its date, where a day is picked
      if (primitive.kind === 'dateField')
        dropArrow.addEventListener('click', () => {
          if (input.disabled || layer._closeOpenPart(element)) return;
          const close = layer._openUnder(
            element,
            layer._calendar(primitive.date, (picked: string) => {
              close();
              layer._send(id, 'date', picked);
              app.map.focus();
            }),
          );
        });
    }

    /*
      Opens a part under the control, a list of entries or a calendar, over the controls below
      it. A click elsewhere or Escape closes it, also with the keyboard outside the field.
      Answers the function that closes it.
    */
    private static _openUnder(
      element: HTMLElement,
      part: HTMLElement,
    ): () => void {
      part.classList.add('vector-form-open-part');
      Object.assign(part.style, {
        position: 'absolute',
        left: '0',
        top: '100%',
        minWidth: '100%',
        boxSizing: 'border-box',
        border: '1px solid var(--color-border-input-dark)',
        backgroundColor: 'var(--color-background-lighter)',
      });
      const close = () => {
        part.remove();
        element.style.zIndex = '';
        window.removeEventListener('pointerdown', closeOutside, true);
        window.removeEventListener('keydown', closeByEscape, true);
      };
      const closeOutside = (event: Event) => {
        if (!element.contains(event.target as Node)) close();
      };
      const closeByEscape = (event: KeyboardEvent) => {
        if (event.key !== 'Escape') return;
        event.stopPropagation();
        close();
      };
      element.style.zIndex = '1';
      element.appendChild(part);
      window.addEventListener('pointerdown', closeOutside, true);
      window.addEventListener('keydown', closeByEscape, true);
      (part as HTMLElement & { closePart?: () => void }).closePart = close;
      return close;
    }

    // closes the part open under the control, answering whether there was one
    private static _closeOpenPart(element: HTMLElement): boolean {
      const open = element.querySelector('.vector-form-open-part') as
        | (HTMLElement & { closePart?: () => void })
        | null;
      open?.closePart?.();
      return !!open;
    }

    /*
      A calendar of a month, the one of the date given as YYYY-MM-DD, or of today: the month
      with arrows to the one before and after, the days of the week, and the days, the given
      one marked. A click on a day answers it as YYYY-MM-DD.
    */
    private static _calendar(
      given: string | undefined,
      onPick: (picked: string) => void,
    ): HTMLElement {
      const parts = (given ?? '').split('-').map(Number);
      const chosen = parts.length === 3 && parts[0] ? parts : undefined;
      const today = new Date();
      let year = chosen ? chosen[0] : today.getFullYear();
      let month = chosen ? chosen[1] - 1 : today.getMonth();
      const language = navigator.language;
      const calendar = (
        <div class="jsdialog" style={{ padding: '2px', fontSize: '0.9em' }} />
      );
      const two = (n: number) => (n < 10 ? '0' : '') + n;

      const show = () => {
        calendar.textContent = '';
        const first = new Date(year, month, 1);
        const step = (by: number) => {
          month += by;
          year += Math.floor(month / 12);
          month = ((month % 12) + 12) % 12;
          show();
        };
        const cell = {
          textAlign: 'center',
          padding: '1px 3px',
          cursor: 'pointer',
        };
        calendar.appendChild(
          <div style={{ display: 'flex', justifyContent: 'space-between' }}>
            <span style={cell} onClick={() => step(-1)}>
              {'\u25C2'}
            </span>
            <span>
              {first.toLocaleDateString(language, {
                month: 'long',
                year: 'numeric',
              })}
            </span>
            <span style={cell} onClick={() => step(1)}>
              {'\u25B8'}
            </span>
          </div>,
        );

        // the week starts on Monday
        const grid = (
          <div
            style={{ display: 'grid', gridTemplateColumns: 'repeat(7, auto)' }}
          />
        );
        for (let day = 0; day < 7; ++day)
          grid.appendChild(
            <span
              style={{
                textAlign: 'center',
                padding: '1px 3px',
                opacity: '0.6',
              }}
            >
              {new Date(2024, 0, 1 + day).toLocaleDateString(language, {
                weekday: 'narrow',
              })}
            </span>,
          );
        for (let gap = (first.getDay() + 6) % 7; gap > 0; --gap)
          grid.appendChild(<span />);
        const days = new Date(year, month + 1, 0).getDate();
        for (let day = 1; day <= days; ++day) {
          const isChosen =
            !!chosen &&
            chosen[0] === year &&
            chosen[1] === month + 1 &&
            chosen[2] === day;
          grid.appendChild(
            <span
              style={
                isChosen
                  ? {
                      ...cell,
                      backgroundColor: 'var(--color-primary)',
                      color: 'var(--color-primary-text)',
                    }
                  : cell
              }
              onClick={() =>
                onPick(year + '-' + two(month + 1) + '-' + two(day))
              }
            >
              {String(day)}
            </span>,
          );
        }
        calendar.appendChild(grid);
      };
      show();
      return calendar;
    }

    /*
      A list that drops down: a list of the browser's own lies over the drawn field, unseen, so
      a click on the field opens it, and the entry chosen there is selected.
    */
    private static _makeDropDownList(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const chosen = (primitive.selected ?? [])[0] ?? -1;
      const list = (
        <select
          class="vector-form-field"
          style={{
            position: 'absolute',
            left: '0',
            top: '0',
            width: '100%',
            height: '100%',
            opacity: '0',
            cursor: 'pointer',
          }}
        >
          {(primitive.items ?? []).map((item: string, at: number) => (
            <option value={String(at)}>{item}</option>
          ))}
        </select>
      ) as HTMLSelectElement;
      list.selectedIndex = chosen;
      if (primitive.disabled) list.disabled = true;
      list.addEventListener('change', () => {
        VectorFormControlLayer._send(id, 'select', String(list.selectedIndex));
        app.map.focus();
      });
      list.addEventListener('keydown', (event: Event) =>
        event.stopPropagation(),
      );
      element.appendChild(list);
    }

    /*
      A list that shows its entries: a click selects the entry alone; with Ctrl, in a list that
      takes several, the entry joins the selection or leaves it.
    */
    private static _makeList(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      if (primitive.disabled) return;
      const selected = primitive.selected ?? [];
      element
        .querySelectorAll<HTMLElement>('.ui-listbox > div')
        .forEach((entry: HTMLElement, at: number) => {
          entry.style.cursor = 'pointer';
          entry.addEventListener('click', (event: MouseEvent) => {
            const toggling =
              primitive.multiSelection && (event.ctrlKey || event.metaKey);
            const wanted = !toggling
              ? [at]
              : selected.includes(at)
                ? selected.filter((place: number) => place !== at)
                : selected.concat([at]);
            VectorFormControlLayer._send(id, 'select', wanted.join(','));
          });
        });
    }

    // where the value of a scroll bar or a spin button goes, kept within its bounds
    private static _moveTo(
      id: number,
      primitive: FormControlPrimitive,
      value: number,
    ): void {
      const min = primitive.min ?? 0;
      const max = primitive.max ?? 100;
      const within = Math.round(Math.min(max, Math.max(min, value)));
      if (within !== (primitive.value ?? min))
        VectorFormControlLayer._send(id, 'value', String(within));
    }

    /// A spin button: up, or right where it runs across, adds a step, the other arrow takes one.
    private static _makeSpinButton(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      if (primitive.disabled) return;
      const arrows = element.firstElementChild?.children;
      if (!arrows || arrows.length < 2) return;
      const step = primitive.step ?? 1;
      const value = primitive.value ?? primitive.min ?? 0;
      const first = primitive.vertical ? step : -step;
      (arrows[0] as HTMLElement).addEventListener('click', () =>
        VectorFormControlLayer._moveTo(id, primitive, value + first),
      );
      (arrows[1] as HTMLElement).addEventListener('click', () =>
        VectorFormControlLayer._moveTo(id, primitive, value - first),
      );
    }

    /*
      A scroll bar: its arrows go a step, a click on the track before or after the thumb a page,
      and the thumb is dragged to where the value goes, which is sent when it is let go.
    */
    private static _makeScrollBar(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      if (primitive.disabled) return;
      const parts = element.firstElementChild?.children;
      if (!parts || parts.length < 3) return;
      const before = parts[0] as HTMLElement;
      const track = parts[1] as HTMLElement;
      const after = parts[2] as HTMLElement;
      const thumb = track.firstElementChild as HTMLElement | null;
      if (!thumb) return;

      const layer = VectorFormControlLayer;
      const vertical = primitive.vertical === true;
      const min = primitive.min ?? 0;
      const max = primitive.max ?? 100;
      const value = primitive.value ?? min;
      const step = primitive.step ?? 1;
      const pageStep =
        primitive.pageStep ??
        primitive.visible ??
        Math.max(1, (max - min) / 10);

      before.addEventListener('click', () =>
        layer._moveTo(id, primitive, value - step),
      );
      after.addEventListener('click', () =>
        layer._moveTo(id, primitive, value + step),
      );
      track.addEventListener('click', (event: MouseEvent) => {
        if (event.target === thumb) return;
        const box = thumb.getBoundingClientRect();
        const ahead = vertical
          ? event.clientY > box.bottom
          : event.clientX > box.right;
        layer._moveTo(id, primitive, value + (ahead ? pageStep : -pageStep));
      });

      // the thumb follows the pointer along the track, from where it was taken hold of
      thumb.addEventListener('pointerdown', (event: PointerEvent) => {
        event.preventDefault();
        event.stopPropagation();
        const trackBox = track.getBoundingClientRect();
        const thumbBox = thumb.getBoundingClientRect();
        const along = (e: PointerEvent) => (vertical ? e.clientY : e.clientX);
        const trackStart = vertical ? trackBox.top : trackBox.left;
        const travel =
          (vertical
            ? trackBox.height - thumbBox.height
            : trackBox.width - thumbBox.width) || 1;
        const grab = along(event) - (vertical ? thumbBox.top : thumbBox.left);
        const shareAt = (e: PointerEvent) =>
          Math.min(1, Math.max(0, (along(e) - grab - trackStart) / travel));
        const travelPercent =
          100 -
          (parseFloat(vertical ? thumb.style.height : thumb.style.width) || 0);
        thumb.setPointerCapture(event.pointerId);
        const move = (e: PointerEvent) => {
          const placed = shareAt(e) * travelPercent + '%';
          if (vertical) thumb.style.top = placed;
          else thumb.style.left = placed;
        };
        const up = (e: PointerEvent) => {
          thumb.removeEventListener('pointermove', move);
          thumb.removeEventListener('pointerup', up);
          layer._moveTo(id, primitive, min + shareAt(e) * (max - min));
        };
        thumb.addEventListener('pointermove', move);
        thumb.addEventListener('pointerup', up);
      });
    }

    private static _send(id: number, action: string, value: string): void {
      app.socket.sendMessage(
        'formcontrol id=' +
          id +
          ' action=' +
          action +
          ' value=' +
          encodeURIComponent(value),
      );
    }
  }
}
