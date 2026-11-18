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

    /// Whether a change of the view's permission draws the page anew.
    private static _hearsPermission = false;

    // the element of each control by its object id, with what it was built from: the
    // primitive and its JSON, and whether the element has sent a change since it was built
    private static _elements = new Map<
      number,
      {
        built: string;
        primitive: FormControlPrimitive;
        element: HTMLElement;
        sent: boolean;
      }
    >();

    /// The ids of the controls shown, in the order of the page.
    private static _shownOrder: number[] = [];

    /// Whether the first control takes the focus the next time the controls are shown, where
    /// the document asks for it: after the document opens and after the design mode ends.
    private static _autoFocusPending = true;

    /// Whether the design mode the controls are shown after is one the document reported, as
    /// against the time before the document reports the mode while it opens.
    private static _leavingDesignMode = false;

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
      const order: number[] = [];
      layer.shownPart = data ? String(data.partId) : '';
      const designMode = app.map.isFormControlDesignMode();
      if (designMode) {
        layer._autoFocusPending = true;
        if (app.map.areFormControlsObjects()) layer._leavingDesignMode = true;
      }
      if (data && !designMode) {
        for (const id of data.order) {
          const object = data.objects.get(id);
          if (!object || object.objectKind !== layer.formControlKind) continue;
          if (
            object.layer !== undefined &&
            !RenderManager.isLayerVisible(object.layer)
          )
            continue;
          const primitive = layer._primitiveIn(object.primitives ?? []);
          if (!primitive) continue;
          shown.add(id);
          order.push(id);
          layer._place(id, primitive, originX, originY, scale);
        }
      }
      layer._shownOrder = order;

      for (const [id, entry] of layer._elements)
        if (!shown.has(id)) {
          layer._closeOpenPart(entry.element);
          entry.element.remove();
          layer._elements.delete(id);
        }

      if (data && !designMode && layer._autoFocusPending)
        layer._focusFirstControl();
    }

    /*
      Gives the first control of the page in the order of Tab the focus, where the document asks
      for that, as the office does when the document opens and when the design mode ends. The
      first control is the first one that takes the keyboard and is enabled. The question waits
      until the document has said whether it asks. Where no control takes the focus after the
      design mode ends, the document has the keyboard.
    */
    private static _focusFirstControl(): void {
      const layer = VectorFormControlLayer;
      const setting = app.map['stateChangeHandler'].getItemValue(
        '.uno:AutoControlFocus',
      );
      if (setting === undefined) return;
      layer._autoFocusPending = false;
      const leavingDesignMode = layer._leavingDesignMode;
      layer._leavingDesignMode = false;
      const first =
        setting === true || setting === 'true'
          ? layer
              ._inTabOrder()
              .find((id: number) => layer._takesFocus(id, false))
          : undefined;
      if (first !== undefined) layer._focusControl(first);
      else if (leavingDesignMode) app.map.focus();
    }

    // the ids of the controls shown, in the order Tab goes through them
    private static _inTabOrder(): number[] {
      const layer = VectorFormControlLayer;
      const placeOf = (id: number) =>
        layer._elements.get(id)?.primitive.tabIndex || Infinity;
      // the sort keeps the order of the page among the controls of one place
      return layer._shownOrder.slice().sort((one: number, other: number) => {
        const onePlace = placeOf(one);
        const otherPlace = placeOf(other);
        return onePlace === otherPlace ? 0 : onePlace < otherPlace ? -1 : 1;
      });
    }

    /*
      Whether that control takes the focus from the keyboard: it is enabled and its element has
      a part that takes the keyboard. Where Tab goes, the control is also one Tab stops at, as its
      model says, or else as its kind does.
    */
    private static _takesFocus(id: number, byTab: boolean): boolean {
      const layer = VectorFormControlLayer;
      const entry = layer._elements.get(id);
      if (!entry || entry.primitive.disabled) return false;
      if (byTab && entry.primitive.tabStop === false) return false;
      // the table control and the navigation bar are drawn and do not work in the client
      const kind = entry.primitive.kind;
      if (kind === 'tableControl' || kind === 'navigationBar') return false;
      return !!layer._focusTarget(entry.element);
    }

    // the part of the control's element that takes the keyboard, where it has one that is enabled
    private static _focusTarget(element: HTMLElement): HTMLElement | null {
      return element.querySelector<HTMLElement>(
        '.vector-form-field:not(:disabled), .vector-form-list, ' +
          'input[type="checkbox"]:not(:disabled), input[type="radio"]:not(:disabled), ' +
          'button.ui-pushbutton:not(:disabled)',
      );
    }

    // gives that control the focus
    private static _focusControl(id: number): void {
      const entry = VectorFormControlLayer._elements.get(id);
      const target = entry
        ? VectorFormControlLayer._focusTarget(entry.element)
        : null;
      target?.focus({ preventScroll: true });
    }

    /*
      Tab moves the focus to the next control in the order of Tab, Shift+Tab to the one before.
      From the last control it goes round to the first one, and from the first to the last one.
    */
    private static _moveFocus(fromId: number, forward: boolean): void {
      const layer = VectorFormControlLayer;
      const order = layer._inTabOrder();
      const from = order.indexOf(fromId);
      if (from < 0) return;
      for (let step = 1; step < order.length; step++) {
        const at =
          (from + (forward ? step : -step) + order.length) % order.length;
        if (layer._takesFocus(order[at], true)) {
          layer._focusControl(order[at]);
          return;
        }
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

      // A view that cannot edit shows every control as a read-only one. A changed control, or
      // a change of the view's permission, makes the element anew. The page sends its objects
      // again unchanged as well.
      const shown = app.map.isEditMode()
        ? primitive
        : { ...primitive, readOnly: true };
      const built = JSON.stringify(shown);
      let entry = layer._elements.get(id);
      // An element that sent a change is made anew once the server sends the control again,
      // also unchanged, so it shows the value the control holds where the server refused or
      // reformatted what was typed. It waits while it has the focus or a part of it is open.
      const answered =
        entry?.sent === true &&
        entry.primitive !== primitive &&
        !entry.element.contains(document.activeElement) &&
        !entry.element.querySelector('.vector-form-open-part');
      if (!entry || entry.built !== built || answered) {
        // the focus stays on the control when its element is made anew
        const focused =
          entry && entry.element.contains(document.activeElement)
            ? (document.activeElement as HTMLElement)
            : null;
        if (entry) {
          layer._closeOpenPart(entry.element);
          entry.element.remove();
        }
        entry = {
          built: built,
          primitive: primitive,
          element: layer._make(id, shown),
          sent: false,
        };
        layer._elements.set(id, entry);
        layer._holderElement().appendChild(entry.element);
        if (focused) layer._focusAgain(id, focused);
      } else if (!answered && !entry.sent) {
        // the control as the server sent it last, so a later change waits for a newer one
        entry.primitive = primitive;
      }

      layer._showImageOf(entry.element, primitive);

      // the element has its size at a zoom of one and is scaled to the view's zoom
      const origin = matrix.apply(0, 0);
      const style = entry.element.style;
      style.left = (originX + origin.x * scale) / app.dpiScale + 'px';
      style.top = (originY + origin.y * scale) / app.dpiScale + 'px';
      style.transform =
        'scale(' + (scale * layer.twipsPerPixel) / app.dpiScale + ')';
    }

    /*
      Gives the focus to the control made anew whose element before had it, at the part that
      takes the keyboard. A field that shows the same text keeps the caret and the selection. A
      control that no longer takes the focus gives the keyboard to the document.
    */
    private static _focusAgain(id: number, focused: HTMLElement): void {
      const layer = VectorFormControlLayer;
      const entry = layer._elements.get(id);
      const target = entry ? layer._focusTarget(entry.element) : null;
      if (!target || !layer._takesFocus(id, false)) {
        app.map.focus();
        return;
      }
      target.focus({ preventScroll: true });
      // only a text input and a text area have a caret to put back
      const hasCaret = (element: HTMLElement) =>
        element instanceof HTMLTextAreaElement ||
        (element instanceof HTMLInputElement && element.type === 'text');
      if (!hasCaret(target)) return;
      const field = target as HTMLInputElement;
      const before = focused as HTMLInputElement;
      if (hasCaret(focused) && before.value === field.value)
        field.setSelectionRange(
          before.selectionStart,
          before.selectionEnd,
          before.selectionDirection ?? undefined,
        );
      // a field that shows another text, as the server formatted it, has the caret at its end
      else field.setSelectionRange(field.value.length, field.value.length);
    }

    /*
      The picture of an image control or an image button, in the element over the control. The
      page leaves the control out while it works as a control, so the element shows the picture
      itself. It fits the control as its scale mode says: at its own size, as large as fits with
      its proportions kept, or stretched. The picture shows once it has arrived from the bitmap
      cache, which the elements are placed again after.
    */
    private static _showImageOf(
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      if (primitive.kind !== 'imageControl' && primitive.kind !== 'imageButton')
        return;
      if (primitive.checksum === undefined) return;
      let shown = element.querySelector<HTMLImageElement>(
        'img.vector-form-image',
      );
      if (shown) return;
      const image = RenderManager.imageFor(primitive.checksum);
      if (!image || !image.complete || !image.naturalWidth) return;

      const scaleMode = FormControlPrimitive.imageScaleMode(primitive);
      // a copy of the image that arrived from the bitmap cache, with its data
      shown = image.cloneNode() as HTMLImageElement;
      shown.className = 'vector-form-image';
      shown.alt = '';
      Object.assign(shown.style, {
        position: 'absolute',
        left: '0',
        top: '0',
        width: '100%',
        height: '100%',
        objectFit: ['none', 'contain', 'fill'][scaleMode] ?? 'fill',
        pointerEvents: 'none',
      });
      element.appendChild(shown);
    }

    // the element over the canvas that holds the controls; it lets the pointer through
    // where no control is
    private static _holderElement(): HTMLElement {
      const layer = VectorFormControlLayer;
      // the controls of a view that cannot edit are read-only, so a change of the view's
      // permission draws the page anew and places the controls again
      if (!layer._hearsPermission) {
        layer._hearsPermission = true;
        app.events.on('updatepermission', () =>
          app.sectionContainer?.requestReDraw(),
        );
      }
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
          // the control works as a text field that holds the path, and the field takes the
          // whole width
          element.querySelector('button')?.remove();
          layer._makeTypable(id, element, primitive);
          break;
        case 'spinButton':
          layer._makeSpinButton(id, element, primitive);
          break;
        case 'imageControl':
          layer._makeImageControl(id, element, primitive);
          break;
        case 'scrollBar':
          layer._makeScrollBar(id, element, primitive);
          break;
      }

      // The Tab of the browser does not go into the controls from the rest of the page. A click
      // gives the focus to a field, as it is typed into, to a list, and to a check box and an
      // option button, also on the label. A click on a button leaves the focus where it was.
      element
        .querySelectorAll<HTMLElement>('button, input, select, textarea, a')
        .forEach((focusable: HTMLElement) =>
          focusable.setAttribute('tabindex', '-1'),
        );
      element.addEventListener('mousedown', (event: MouseEvent) => {
        if ((event.target as Element).classList.contains('vector-form-field'))
          return;
        event.preventDefault();
        const isBox =
          primitive.kind === 'checkBox' || primitive.kind === 'optionButton';
        if (isBox && !primitive.disabled)
          layer._focusTarget(element)?.focus({ preventScroll: true });
      });

      // In a control with the focus, Tab and Shift+Tab go to the next and the previous control,
      // before the control itself sees the key, and close a part open under the control. Escape
      // gives the keyboard to the document. A field that is typed into takes Escape itself, as it
      // takes back the text first.
      element.addEventListener(
        'keydown',
        (event: KeyboardEvent) => {
          if (event.ctrlKey || event.altKey || event.metaKey) return;
          if (event.key === 'Tab') {
            event.preventDefault();
            event.stopPropagation();
            layer._closeOpenPart(element);
            layer._moveFocus(id, !event.shiftKey);
          } else if (
            event.key === 'Escape' &&
            !(event.target as Element).matches(
              'input.vector-form-field, textarea.vector-form-field',
            )
          ) {
            event.preventDefault();
            event.stopPropagation();
            app.map.focus();
          }
        },
        true,
      );
      // the keys typed in a control are the control's, the document does not see them
      for (const type of ['keydown', 'keypress', 'keyup'])
        element.addEventListener(type, (event: Event) =>
          event.stopPropagation(),
        );

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

      // a read-only check box or option button keeps its state when it is clicked
      if (primitive.readOnly)
        element
          .querySelectorAll<HTMLInputElement>(
            'input[type="checkbox"], input[type="radio"]',
          )
          .forEach((box: HTMLInputElement) =>
            box.addEventListener('click', (event: Event) =>
              event.preventDefault(),
            ),
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
      several lines. Return, or leaving the field, sends what was typed, and the field keeps the
      focus after Return. Escape takes back what was typed and gives the keyboard back to the
      document. The keys typed there are the field's, the document does not see them.
    */
    private static _makeTypable(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const shown = element.querySelector('.ui-edit')?.firstElementChild;
      if (!shown) return;

      // A time of day is typed and picked in the browser's time input, which shows the hours and
      // the minutes to fill in and writes the time as HH:MM or HH:MM:SS in every language.
      const isClock = primitive.kind === 'timeField' && !primitive.duration;
      const original = isClock
        ? (primitive.time ?? '').slice(0, primitive.seconds ? 8 : 5)
        : (primitive.text ?? '');
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
      if (isClock) {
        const clock = input as HTMLInputElement;
        clock.type = 'time';
        clock.step = primitive.seconds ? '1' : '60';
      }
      input.value = original;
      if (primitive.disabled) input.disabled = true;
      if (primitive.readOnly) input.readOnly = true;
      shown.replaceWith(input);

      // The time input reports a change for each part of the time that is typed, so the time
      // goes to the server when the field is left or a spin button steps it. An emptied input
      // empties the field, and a time left half typed goes back to the last one sent. The input
      // writes HH:MM where it shows no seconds or they are zero, and a format without seconds
      // keeps the seconds the field holds.
      let sentTime = input.value;
      const sendTime = () => {
        // a field made anew from a newer value of the control sends nothing any more
        if (!input.isConnected) return;
        if (!input.value && (input as HTMLInputElement).validity.badInput) {
          input.value = sentTime;
          return;
        }
        if (input.value === sentTime) return;
        sentTime = input.value;
        if (!input.value) {
          VectorFormControlLayer._send(id, 'text', '');
          return;
        }
        const seconds = primitive.seconds
          ? ':00'
          : (primitive.time ?? '').slice(5, 8) || ':00';
        VectorFormControlLayer._send(
          id,
          'time',
          input.value.length === 5 ? input.value + seconds : input.value,
        );
      };
      // The other fields send their text when it differs from the text the server last got.
      let sentText = original;
      const sendText = () => {
        if (!input.isConnected || input.value === sentText) return;
        sentText = input.value;
        VectorFormControlLayer._send(id, 'text', input.value);
      };
      if (isClock) input.addEventListener('blur', sendTime);
      else input.addEventListener('change', sendText);
      input.addEventListener('keydown', (event: Event) => {
        event.stopPropagation();
        const key = (event as KeyboardEvent).key;
        if (key === 'Escape') {
          // Escape takes back what was typed since the value the server last got
          input.value = isClock ? sentTime : sentText;
          event.preventDefault();
          input.blur();
          app.map.focus();
        } else if (key === 'Enter' && !primitive.multiLine) {
          event.preventDefault();
          if (isClock) sendTime();
          else sendText();
        }
      });
      for (const type of ['keypress', 'keyup', 'input'])
        input.addEventListener(type, (event: Event) => event.stopPropagation());

      const layer = VectorFormControlLayer;
      const kind = primitive.kind;
      if (
        kind === 'numericField' ||
        kind === 'currencyField' ||
        kind === 'dateField' ||
        (kind === 'timeField' && !isClock)
      ) {
        if (kind === 'numericField' || kind === 'currencyField')
          input.setAttribute('inputmode', 'decimal');
        input.addEventListener('beforeinput', (event: Event) => {
          const inputEvent = event as InputEvent;
          if (inputEvent.isComposing) return;
          const typed =
            inputEvent.data ??
            inputEvent.dataTransfer?.getData('text/plain') ??
            '';
          if (
            !Array.from(typed).every((character: string) =>
              layer._takesCharacter(primitive, character),
            )
          )
            event.preventDefault();
        });
        // What an input method composes is checked when the composition ends. Text the field
        // does not take leaves the field as it was before.
        let beforeComposing: string | undefined;
        input.addEventListener('compositionstart', () => {
          beforeComposing = input.value;
        });
        input.addEventListener('compositionend', (event: Event) => {
          const composed = (event as CompositionEvent).data ?? '';
          if (
            beforeComposing !== undefined &&
            !Array.from(composed).every((character: string) =>
              layer._takesCharacter(primitive, character),
            )
          )
            input.value = beforeComposing;
          beforeComposing = undefined;
        });
      }
      if (
        kind === 'patternField' &&
        primitive.strictFormat &&
        primitive.editMask &&
        !primitive.multiLine
      )
        layer._typeByMask(
          input as HTMLInputElement,
          primitive.editMask,
          primitive.literalMask ?? '',
        );

      const extras = Array.from(input.parentElement?.children ?? []).slice(1);
      const dropArrow = primitive.dropdown
        ? (extras[0] as HTMLElement | undefined)
        : undefined;
      const spinArrows = primitive.spin
        ? extras[primitive.dropdown ? 1 : 0]?.children
        : undefined;

      // the spin buttons step the value as the field's format says; what was typed goes first.
      // The time input steps its minutes, or its seconds where it shows them, itself.
      const spin = (up: boolean) => {
        if (input.disabled || input.readOnly) return;
        if (isClock) {
          const clock = input as HTMLInputElement;
          // a time left half typed goes back to the last one sent before it is stepped
          if (!clock.value && clock.validity.badInput) clock.value = sentTime;
          if (up) clock.stepUp();
          else clock.stepDown();
          sendTime();
          return;
        }
        sendText();
        layer._send(id, 'spin', up ? 'up' : 'down');
      };
      if (spinArrows && spinArrows.length >= 2) {
        (spinArrows[0] as HTMLElement).addEventListener('click', () =>
          spin(true),
        );
        (spinArrows[1] as HTMLElement).addEventListener('click', () =>
          spin(false),
        );
        // the arrow keys of the time input step the part of the time the caret is in
        input.addEventListener('keydown', (event: Event) => {
          const key = (event as KeyboardEvent).key;
          if (isClock || (key !== 'ArrowUp' && key !== 'ArrowDown')) return;
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
          if (input.disabled || input.readOnly || layer._closeOpenPart(element))
            return;
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
                    sendText();
                    // the field keeps the focus after the pick, or takes it
                    input.focus({ preventScroll: true });
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
          if (input.disabled || input.readOnly || layer._closeOpenPart(element))
            return;
          const close = layer._openUnder(
            element,
            layer._calendar(primitive.date, (picked: string) => {
              close();
              layer._send(id, 'date', picked);
              // the field keeps the focus after the pick, or takes it
              input.focus({ preventScroll: true });
            }),
          );
        });
    }

    /*
      Whether a field of numbers, amounts, dates or times takes that typed character. It takes
      digits, signs, separators and spaces, and the other characters of the text it shows. An
      amount takes every currency symbol, and a date or a time takes every letter, as its format
      can write a month or AM as a word. This is a help in typing: the server checks the value
      the field keeps.
    */
    private static _takesCharacter(
      primitive: FormControlPrimitive,
      character: string,
    ): boolean {
      if (/[\d\s+\-.,:;'/]/.test(character)) return true;
      if ((primitive.text ?? '').includes(character)) return true;
      if (primitive.kind === 'currencyField' && /\p{Sc}/u.test(character))
        return true;
      const isDateOrTime =
        primitive.kind === 'dateField' || primitive.kind === 'timeField';
      return isDateOrTime && /\p{L}/u.test(character);
    }

    /*
      Typing in a pattern field fills the positions of its mask in turn, as the field does on the
      desktop. A typed character takes the place of the next position that is not fixed, where
      the mask allows that character, and the caret then jumps over the fixed characters. A
      character the mask does not allow there is dropped, unless it is the next fixed character,
      which moves the caret past it. Deleting puts back what the literal mask shows at that
      position, so the text keeps the length of the mask. What an input method composes goes
      through the mask as typed text when the composition ends.
    */
    private static _typeByMask(
      input: HTMLInputElement,
      editMask: string,
      literalMask: string,
    ): void {
      const length = editMask.length;
      // A literal mask of another length is cut or filled with spaces to the edit mask.
      const literal = literalMask.slice(0, length).padEnd(length, ' ');
      const isFixed = (position: number) => editMask[position] === 'L';
      const nextFree = (position: number) => {
        while (position < length && isFixed(position)) position++;
        return position;
      };
      const takes = (character: string, maskCharacter: string) => {
        if (character.length !== 1) return false;
        switch (maskCharacter) {
          case 'a':
          case 'A':
            return /\p{L}/u.test(character);
          case 'N':
            return /\p{Nd}/u.test(character);
          case 'n':
            return /\p{Nd}/u.test(character) || character === ' ';
          case 'c':
          case 'C':
            return /[\p{L}\p{Nd}]/u.test(character);
          case 'x':
          case 'X':
            return character >= ' ';
        }
        return false;
      };

      // The text before an input method began to compose, while it composes.
      let beforeComposing:
        { value: string; start: number; end: number } | undefined;

      // The value the server last got, so leaving the field sends a changed value once. A
      // composition still open when the field is left is dropped, as the mask has not seen it.
      let committed = input.value;
      input.addEventListener('focus', () => (committed = input.value));
      input.addEventListener('change', () => (committed = input.value));
      input.addEventListener('blur', () => {
        if (beforeComposing) {
          input.value = beforeComposing.value;
          beforeComposing = undefined;
        }
        if (input.value !== committed) input.dispatchEvent(new Event('change'));
      });

      // Edits the text by the mask: an insertion or a deletion of that input type, of the typed
      // text, over the selection from start to end.
      const edit = (
        type: string,
        typed: string,
        start: number,
        end: number,
      ) => {
        // one position of the mask is one UTF-16 unit, as the caret counts
        const text = (
          input.value.slice(0, length) + literal.slice(input.value.length)
        ).split('');
        start = Math.min(start, length);
        end = Math.min(end, length);
        const before = text.slice();
        // a selection goes back to what the literal mask shows
        for (let position = start; position < end; position++)
          text[position] = literal[position];
        let caret = start;
        let taken = 0;

        if (type.startsWith('delete')) {
          if (start === end && type.includes('Backward')) {
            let position = start - 1;
            while (position >= 0 && isFixed(position)) position--;
            if (position >= 0) {
              text[position] = literal[position];
              caret = position;
            }
          } else if (start === end) {
            const position = nextFree(start);
            if (position < length) {
              text[position] = literal[position];
              caret = position;
            }
          }
        } else {
          for (const character of Array.from(typed)) {
            const position = nextFree(caret);
            if (position >= length) break;
            const maskCharacter = editMask[position];
            if (takes(character, maskCharacter)) {
              const upper = character.toLocaleUpperCase();
              text[position] =
                /[ACX]/.test(maskCharacter) && upper.length === 1
                  ? upper
                  : character;
              caret = position + 1;
              taken++;
              continue;
            }
            // the next fixed character, typed, moves the caret past it, also where the caret
            // stands right before it
            let fixed = caret;
            while (fixed < length && !isFixed(fixed)) fixed++;
            if (fixed < length && character === literal[fixed])
              caret = fixed + 1;
          }
          caret = Math.min(nextFree(caret), length);
          // typing that the mask takes nothing of leaves a selection as it was
          if (!taken && start !== end) {
            input.value = before.join('');
            input.setSelectionRange(start, end);
            return;
          }
        }

        input.value = text.join('');
        input.setSelectionRange(caret, caret);
      };

      input.addEventListener('beforeinput', (event: Event) => {
        const inputEvent = event as InputEvent;
        const type = inputEvent.inputType;
        // what an input method composes is taken when the composition ends
        if (inputEvent.isComposing || type.includes('Composition')) return;
        // The browser's undo knows none of the edits made here, and a drop lands where the
        // pointer is, not at the selection, so the field takes neither.
        if (
          type === 'historyUndo' ||
          type === 'historyRedo' ||
          type === 'insertFromDrop' ||
          type === 'deleteByDrag'
        ) {
          event.preventDefault();
          return;
        }
        if (!type.startsWith('insert') && !type.startsWith('delete')) return;
        event.preventDefault();
        const typed =
          inputEvent.data ??
          inputEvent.dataTransfer?.getData('text/plain') ??
          '';
        const start = input.selectionStart ?? 0;
        edit(type, typed, start, input.selectionEnd ?? start);
      });

      // An input method writes its text into the field while it composes. When it ends, the
      // field goes back to the text before and the composed text is typed by the mask.
      input.addEventListener('compositionstart', () => {
        const start = input.selectionStart ?? 0;
        beforeComposing = {
          value: input.value,
          start: start,
          end: input.selectionEnd ?? start,
        };
      });
      input.addEventListener('compositionend', (event: Event) => {
        if (!beforeComposing) return;
        const { value, start, end } = beforeComposing;
        beforeComposing = undefined;
        input.value = value;
        edit('insertText', (event as CompositionEvent).data ?? '', start, end);
      });
    }

    /*
      An image control takes a picture as it does on the desktop. A double click asks for a
      picture file, which the server asks the browser for, and the context menu offers to insert
      a picture or to delete the one the control shows. The server checks that the file holds a
      picture and keeps it in the document.
    */
    private static _makeImageControl(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const layer = VectorFormControlLayer;
      const takesChanges = !primitive.disabled && !primitive.readOnly;
      if (takesChanges)
        element.addEventListener('dblclick', () =>
          layer._send(id, 'image', 'pick'),
        );
      // a right click opens the control's own menu where it takes changes, and else no menu
      element.addEventListener('contextmenu', (event: Event) => {
        event.preventDefault();
        event.stopPropagation();
        if (!takesChanges || layer._closeOpenPart(element)) return;
        const hasImage = primitive.checksum !== undefined;
        const entry = (text: string, action: string, enabled: boolean) => (
          <div
            style={{
              padding: '0 4px',
              whiteSpace: 'nowrap',
              cursor: enabled ? 'pointer' : 'default',
              opacity: enabled ? '1' : '0.5',
            }}
            onClick={() => {
              if (!enabled) return;
              close();
              layer._send(id, 'image', action);
            }}
          >
            {text}
          </div>
        );
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
            {entry(_('Insert Image...'), 'pick', true)}
            {entry(_('Delete'), 'clear', hasImage)}
          </div>,
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
        (HTMLElement & { closePart?: () => void }) | null;
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
      if (primitive.disabled || primitive.readOnly) list.disabled = true;
      // the list keeps the focus after the pick
      list.addEventListener('change', () =>
        VectorFormControlLayer._send(id, 'select', String(list.selectedIndex)),
      );
      list.addEventListener('keydown', (event: Event) =>
        event.stopPropagation(),
      );
      element.appendChild(list);
    }

    /*
      A list that shows its entries: a click selects the entry alone; with Ctrl, in a list that
      takes several, the entry joins the selection or leaves it. An enabled list takes the focus
      by Tab and by a click, and shows it with an outline. With the focus, the up and down arrows
      select the entry above or below the first one selected, and Home and End the first and the
      last entry.
    */
    private static _makeList(
      id: number,
      element: HTMLElement,
      primitive: FormControlPrimitive,
    ): void {
      const list = element.querySelector<HTMLElement>('.ui-listbox');
      if (!list || primitive.disabled) return;
      list.classList.add('vector-form-list');
      list.setAttribute('tabindex', '-1');
      list.addEventListener('mousedown', () =>
        list.focus({ preventScroll: true }),
      );
      if (primitive.readOnly) return;

      const selected = primitive.selected ?? [];
      const count = primitive.items?.length ?? 0;
      // the entry the keys last selected, so a key pressed again before the server answers goes
      // on from there
      let current = selected.length ? Math.min(...selected) : -1;
      let alone = selected.length === 1;
      list.addEventListener('keydown', (event: KeyboardEvent) => {
        if (event.ctrlKey || event.altKey || event.metaKey || !count) return;
        let wanted: number;
        switch (event.key) {
          case 'ArrowDown':
            wanted = Math.min(count - 1, current + 1);
            break;
          case 'ArrowUp':
            wanted = Math.max(0, current - 1);
            break;
          case 'Home':
            wanted = 0;
            break;
          case 'End':
            wanted = count - 1;
            break;
          default:
            return;
        }
        event.preventDefault();
        if (alone && current === wanted) return;
        current = wanted;
        alone = true;
        VectorFormControlLayer._send(id, 'select', String(wanted));
      });
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
      if (primitive.disabled || primitive.readOnly) return;
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
      if (primitive.disabled || primitive.readOnly) return;
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
      const entry = VectorFormControlLayer._elements.get(id);
      if (entry) entry.sent = true;
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
