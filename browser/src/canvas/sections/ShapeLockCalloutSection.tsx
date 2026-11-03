// @ts-strict-ignore -*- Mode: JavaScript; js-indent-level: 8; fill-column: 100 -*-

/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// The question of the callout. 'request' when the other view asks for the text this user edits, and
// 'giveback' when this user took the text of the other view and can give it back.
type ShapeLockCalloutKind = 'request' | 'giveback';

/*
 * A callout below the bottom right corner of the shape whose text this user edits, in Impress and
 * Draw. It shows one of two questions, each with a countdown and a button:
 *
 * - Another user asks for the text. The button rejects that request and every other one still open.
 *   When the countdown ends, the other user takes the text over.
 * - This user took the text over without a request. The button gives the text back. When the
 *   countdown ends, this user keeps the text.
 */
class ShapeLockCalloutSection extends HTMLObjectSection {
  interactable: boolean = false;
  // The section is a point with no size, so it is drawn wherever that point is, and the callout
  // moves with it.
  isAlwaysVisible: boolean = true;

  static sectionName: string = 'ShapeLockCallout';
  static section: ShapeLockCalloutSection | null = null;
  // The views that asked for the text this user edits, each with the time of its request, in
  // milliseconds. The callout shows the last request. A request stays after the callout closes, as
  // the take over that answers it can arrive after the text edit of this user ends.
  static requests: Map<number, number> = new Map();

  constructor(viewId: number, kind: ShapeLockCalloutKind, objectId: string) {
    super(
      ShapeLockCalloutSection.sectionName,
      null,
      null,
      new cool.SimplePoint(0, 0),
      'shape-lock-callout',
      false,
    );

    const isRequest = kind === 'request';
    const duration = isRequest
      ? ShapeLockLabelSection.approvalTime
      : ShapeLockLabelSection.undoTime;
    // The countdown changes each second, so screen readers leave it out of the alert.
    const countdown = (
      <span class="shape-lock-callout-countdown" aria-hidden="true" />
    ) as HTMLElement;
    const bar = (<i />) as HTMLElement;

    const div = this.getHTMLObject();
    div.setAttribute('role', 'alert');
    if (isRequest)
      div.replaceChildren(
        ShapeLockLabelSection.createAvatar(viewId),
        <span>
          <span class="visuallyhidden">
            {ShapeLockLabelSection.getName(viewId)}
          </span>{' '}
          {_('wants to edit')}
        </span>,
      );
    else
      div.replaceChildren(<span>{_('You took over. Made a mistake?')}</span>);
    div.append(
      countdown,
      <button
        type="button"
        class="button shape-lock-button"
        onClick={() => (isRequest ? this.reject() : this.giveBack())}
      >
        {isRequest ? _('Reject') : _('Cancel')}
      </button>,
      <span class="shape-lock-callout-progress" aria-hidden="true">
        {bar}
      </span>,
    );

    // A press on the callout leaves the keyboard focus on the document, and the map below sees none
    // of its clicks.
    div.style.pointerEvents = 'auto';
    div.addEventListener('mousedown', (event: MouseEvent) =>
      event.preventDefault(),
    );
    for (const type of ['mousedown', 'mouseup', 'click', 'dblclick'])
      div.addEventListener(type, (event: Event) => event.stopPropagation());

    this.sectionProperties.viewId = viewId;
    // The unique id of the shape that a give back is for, or '' for a request.
    this.sectionProperties.objectId = objectId;
    this.sectionProperties.isRequest = isRequest;
    this.sectionProperties.countdown = countdown;
    // The text the countdown shows, or is about to show.
    this.sectionProperties.countdownText = null;
    this.sectionProperties.startTime = Date.now();
    this.sectionProperties.duration = duration;
    // The text cursor of this user showed since the callout opened, so the text edit began. A
    // request comes while this user edits the text. A give back waits for the text cursor of the
    // edit that the take over starts, as this user may leave other text first.
    this.sectionProperties.sawCursor = isRequest && app.file.textCursor.visible;
    this.sectionProperties.tickTimer = app.timerRegistry.setInterval(
      'shapelockcallout',
      () => this.onTick(),
      250,
    );
    this.sectionProperties.pendingTransform = null;
    ShapeLockLabelSection.fillProgress(bar, duration);
  }

  // The right edge of the callout lines up with the right edge of the shape, and the callout sits
  // just below the shape, so the text of the shape stays in view. When the callout would leave the
  // document area on the left, the right or the bottom, it moves to the edge of that area.
  adjustHTMLObjectPosition() {
    super.adjustHTMLObjectPosition();

    if (
      !this.containerObject ||
      !this.containerObject.getDocumentAnchorSection()
    )
      return;

    const anchor = this.containerObject.getDocumentAnchor();
    const anchorSize = this.containerObject.getDocumentAnchorSection().size;
    const left = Math.round(anchor[0] / app.dpiScale) - this.htmlPosition[0];
    const right =
      Math.round((anchor[0] + anchorSize[0]) / app.dpiScale) -
      this.htmlPosition[0];
    const bottom =
      Math.round((anchor[1] + anchorSize[1]) / app.dpiScale) -
      this.htmlPosition[1];
    const transform =
      'translate(max(' +
      left +
      'px, calc(' +
      Math.min(0, right) +
      'px - 100%)), min(4px, calc(' +
      bottom +
      'px - 100%)))';
    ShapeLockLabelSection.writeTransform(this, transform);
  }

  private onTick() {
    const remaining =
      this.sectionProperties.duration -
      (Date.now() - this.sectionProperties.startTime);
    if (remaining <= 0) {
      ShapeLockCalloutSection.close();
      return;
    }

    const text = _('%1 s').replace('%1', String(Math.ceil(remaining / 1000)));
    if (this.sectionProperties.countdownText !== text) {
      this.sectionProperties.countdownText = text;
      const countdown = this.sectionProperties.countdown;
      app.layoutingService.appendLayoutingTask(() => {
        countdown.textContent = text;
      });
    }
    this.updatePosition();
  }

  // Take the place of the bottom right corner of the shape whose text this user edits, or of the
  // text cursor before the shape is known. The callout shows once it has a place.
  private updatePosition() {
    const shape = GraphicSelection.rectangle;
    const cursor = app.file.textCursor;
    let x: number, y: number;
    if (shape && shape.pWidth > 0 && shape.pHeight > 0)
      [x, y] = [shape.pX2, shape.pY2];
    else if (cursor.visible && cursor.rectangle && cursor.rectangle.pHeight > 0)
      [x, y] = [cursor.rectangle.pX2, cursor.rectangle.pY2];
    else return;

    if (
      !this.showSection ||
      Math.round(x) !== this.position[0] ||
      Math.round(y) !== this.position[1]
    ) {
      this.setPosition(x, y);
      this.setShowSection(true);
      this.adjustHTMLObjectPosition();
    }
  }

  // Each view that asked waits for its own answer, so each one gets a rejection.
  private reject() {
    ShapeLockCalloutSection.requests.forEach((time, viewId) => {
      if (ShapeLockCalloutSection.isOpen(time))
        app.map.sendUnoCommand('.uno:RejectTextEditRequest', {
          ViewId: { type: 'long', value: viewId },
        });
    });
    ShapeLockCalloutSection.requests.clear();
    ShapeLockCalloutSection.close();
  }

  private giveBack() {
    app.map.sendUnoCommand('.uno:ReturnTextEdit', {
      ViewId: { type: 'long', value: this.sectionProperties.viewId },
      ObjectId: { type: 'string', value: this.sectionProperties.objectId },
    });
    ShapeLockCalloutSection.close();
  }

  public onRemove(): void {
    app.timerRegistry.clearInterval(this.sectionProperties.tickTimer);
    super.onRemove();
  }

  private static show(
    viewId: number,
    kind: ShapeLockCalloutKind,
    objectId: string = '',
  ) {
    ShapeLockCalloutSection.close();
    const section = new ShapeLockCalloutSection(viewId, kind, objectId);
    ShapeLockCalloutSection.section = section;
    app.sectionContainer.addSection(section);
    section.onTick();
    app.sectionContainer.requestReDraw();
  }

  // The given view asks for the text that this user edits.
  public static showRequest(viewId: number) {
    ShapeLockCalloutSection.requests.set(viewId, Date.now());
    ShapeLockCalloutSection.show(viewId, 'request');
  }

  // A request ends a moment after its countdown, the time a take over that answers it takes to
  // arrive.
  private static isOpen(time: number): boolean {
    return (
      Date.now() - time <
      ShapeLockLabelSection.approvalTime + ShapeLockLabelSection.approvalMargin
    );
  }

  // This user took the text of the given shape over from the given view, and can give it back for
  // a short time.
  public static showGiveBack(viewId: number, objectId: string) {
    ShapeLockCalloutSection.show(viewId, 'giveback', objectId);
  }

  // The given view asked for the text that this user edits, and this user did not reject the
  // request or type since. The text is gone, so no request is open after this.
  public static takeRequestOf(viewId: number): boolean {
    const time = ShapeLockCalloutSection.requests.get(viewId);
    ShapeLockCalloutSection.requests.clear();
    return time !== undefined && ShapeLockCalloutSection.isOpen(time);
  }

  public static close() {
    const section = ShapeLockCalloutSection.section;
    if (!section) return;

    ShapeLockCalloutSection.section = null;
    app.sectionContainer.removeSection(section.name);
    app.sectionContainer.requestReDraw();
  }

  public static closeIfFrom(viewId: number) {
    ShapeLockCalloutSection.requests.delete(viewId);
    const section = ShapeLockCalloutSection.section;
    if (section && section.sectionProperties.viewId === viewId)
      ShapeLockCalloutSection.close();
  }

  // This user typed or moved the text cursor, which ends every request for the text.
  public static onOwnInput() {
    ShapeLockCalloutSection.requests.clear();
    const section = ShapeLockCalloutSection.section;
    if (section && section.sectionProperties.isRequest)
      ShapeLockCalloutSection.close();
  }

  // The callout belongs to the text edit, so it goes away when the text edit ends.
  public static onTextCursorVisibility(event: any) {
    const section = ShapeLockCalloutSection.section;
    if (!section) return;

    if (event.detail.visible) section.sectionProperties.sawCursor = true;
    else if (section.sectionProperties.sawCursor)
      ShapeLockCalloutSection.close();
  }
}

app.events.on(
  'TextCursorVisibility',
  ShapeLockCalloutSection.onTextCursorVisibility.bind(ShapeLockCalloutSection),
);
