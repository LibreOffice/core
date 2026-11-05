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

/*
 * The label on a shape whose text another user edits, in Impress and Draw. The shape stays locked
 * for everyone else until that user leaves the text. The label sits above the top left corner of
 * the colored border around the shape. It shows the avatar of that user and how recently that user
 * typed: editing, active or inactive.
 *
 * Once the other user is active, Take over on the label asks that user for the shape, and this user
 * gets it when that user does not answer in time. Once the other user is inactive, Take over gives
 * the shape to this user at once, and this user can give it back for a short time after.
 */

// How recently the user who edits the text of a shape typed or moved the text cursor.
type ShapeLockPhase = 'editing' | 'active' | 'inactive';

// Another user took over the text that this user edited.
interface ShapeLockTakeOver {
  // That user took the text without a request, and can still give it back.
  canGiveBack: boolean;
  // That user typed or moved the text cursor since then.
  hasInput: boolean;
}

class ShapeLockLabelSection extends HTMLObjectSection {
  interactable: boolean = false;

  static namePrefix: string = 'ShapeLockLabel ';
  static sections: Map<number, ShapeLockLabelSection> = new Map();
  // A user who does not type for this long, in milliseconds, is active, and no longer editing.
  static activeAfter: number = 10000;
  // A user who does not type for this long, in milliseconds, is inactive.
  static inactiveAfter: number = 60000;
  // How long the user who edits the text has to answer a request for it, in milliseconds.
  static approvalTime: number = 20000;
  // How long a take over can take to arrive after it is sent, in milliseconds.
  static approvalMargin: number = 5000;
  // How long a user who took the text over at once can give it back, in milliseconds.
  static undoTime: number = 5000;
  // How long a message in the snackbar shows, in milliseconds.
  static messageTime: number = 6000;

  // The last place of the text cursor of each other view, in twips.
  static cursors: Map<number, string> = new Map();
  // The views that took the text over from this user, by view id.
  static takeOvers: Map<number, ShapeLockTakeOver> = new Map();
  // The text of the last message of this label in the snackbar, or null.
  static messageText: string | null = null;
  static tickTimer: ReturnType<typeof setInterval> | null = null;

  constructor(viewId: number) {
    super(
      ShapeLockLabelSection.namePrefix + viewId,
      null,
      null,
      new cool.SimplePoint(0, 0),
      'shape-lock-label',
    );

    const username = ShapeLockLabelSection.getName(viewId);
    const status = (<span class="shape-lock-label-status" />) as HTMLElement;
    const takeOverButton = (
      <button
        type="button"
        class="button shape-lock-button"
        onClick={() => this.onTakeOverClick()}
      >
        {_('Take over')}
      </button>
    ) as HTMLButtonElement;
    const waitText = (<span />) as HTMLElement;
    const waitBar = (<i />) as HTMLElement;
    const wait = (
      <span class="shape-lock-label-wait">
        {waitText}
        {waitBar}
      </span>
    ) as HTMLElement;

    const div = this.getHTMLObject();
    div.style.backgroundColor = app.LOUtil.rgbToHex(
      app.LOUtil.getViewIdColor(viewId),
    );
    // The buttons sit together in a span at the end of the label.
    div.replaceChildren(
      ShapeLockLabelSection.createAvatar(viewId),
      <span class="visuallyhidden">{username}</span>,
      status,
      <span class="shape-lock-label-buttons">
        {takeOverButton}
        {wait}
      </span>,
    );

    // A press on the label leaves the keyboard focus on the document, and the map below sees none
    // of its clicks.
    div.style.pointerEvents = 'auto';
    div.addEventListener('mousedown', (event: MouseEvent) =>
      event.preventDefault(),
    );
    for (const type of ['mousedown', 'mouseup', 'click', 'dblclick'])
      div.addEventListener(type, (event: Event) => event.stopPropagation());

    this.sectionProperties.viewId = viewId;
    this.sectionProperties.status = status;
    this.sectionProperties.takeOverButton = takeOverButton;
    this.sectionProperties.wait = wait;
    this.sectionProperties.waitText = waitText;
    this.sectionProperties.waitBar = waitBar;
    // The area of the text the user edits, in twips, as the lock message gives it.
    this.sectionProperties.lockRectangle = null;
    // The unique id of the shape, as the lock message gives it.
    this.sectionProperties.objectId = '';
    // The part identifier of the part that holds the shape, as the lock message gives it.
    this.sectionProperties.part = '';
    this.sectionProperties.mode = 0;
    // When the other user last typed or moved the text cursor, in milliseconds.
    this.sectionProperties.lastInput = Date.now();
    // When this user asked the other user for the text, in milliseconds, or null.
    this.sectionProperties.requestTime = null;
    // When this user last sent a take over for the text, in milliseconds, or null.
    this.sectionProperties.takeOverTime = null;
    // What the label shows, or is about to show: statusText, hideTakeOver and countdownText.
    this.sectionProperties.shown = null;
    this.sectionProperties.pendingTransform = null;
  }

  // The label sits above the shape. When the left or the top edge of the shape is out of view, the
  // label moves to the edge of the document area, so it stays on the part of the shape that shows.
  adjustHTMLObjectPosition() {
    super.adjustHTMLObjectPosition();

    if (
      !this.containerObject ||
      !this.containerObject.getDocumentAnchorSection()
    )
      return;

    const anchor = this.containerObject.getDocumentAnchor();
    const left = Math.round(anchor[0] / app.dpiScale) - this.htmlPosition[0];
    const top = Math.round(anchor[1] / app.dpiScale) - this.htmlPosition[1];
    const transform =
      'translate(' +
      Math.max(0, left) +
      'px, max(calc(-100% - 1px), ' +
      top +
      'px))';
    ShapeLockLabelSection.writeTransform(this, transform);
  }

  // Write the transform of the HTML object of the section in the same layout pass as its left and
  // top.
  static writeTransform(section: HTMLObjectSection, transform: string) {
    const properties = section.sectionProperties;
    if (properties.pendingTransform)
      app.layoutingService.cancelLayoutingTask(properties.pendingTransform);
    properties.pendingTransform = app.layoutingService.appendLayoutingTask(
      () => {
        properties.pendingTransform = null;
        const div = section.getHTMLObject();
        if (div.style.transform !== transform) div.style.transform = transform;
      },
    );
  }

  // The index the part that holds the shape has in the current part list, or -1 when no part
  // carries it. The part is kept as its identifier, because the index of a slide changes when
  // slides are inserted, deleted or moved.
  private getPartIndex(): number {
    return app.map._docLayer.getIndexFromPart(this.sectionProperties.part);
  }

  // The shape is on the part and in the mode shown here.
  private checkMyVisibility(): boolean {
    return (
      this.getPartIndex() === app.map._docLayer._selectedPart &&
      app.activeDocument.isModeActive(this.sectionProperties.mode)
    );
  }

  private getPhase(now: number): ShapeLockPhase {
    const idleTime = now - this.sectionProperties.lastInput;
    if (idleTime < ShapeLockLabelSection.activeAfter) return 'editing';
    if (idleTime < ShapeLockLabelSection.inactiveAfter) return 'active';
    return 'inactive';
  }

  private getName(): string {
    return ShapeLockLabelSection.getName(this.sectionProperties.viewId);
  }

  // Show the phase of the other user, and the time this user still waits for an answer. The label
  // changes in a layouting task, once what it shows changes.
  private render(now: number) {
    const phase = this.getPhase(now);
    const statusText =
      phase === 'editing'
        ? _('is editing')
        : phase === 'active'
          ? _('is active')
          : _('is inactive');

    const requestTime = this.sectionProperties.requestTime;
    // A user who cannot edit the document gets no Take over.
    const hideTakeOver =
      phase === 'editing' || requestTime !== null || !app.map.isEditMode();
    let countdownText: string | null = null;
    if (requestTime !== null) {
      const waitTime = Math.max(
        0,
        ShapeLockLabelSection.approvalTime - (now - requestTime),
      );
      countdownText = _('Take over in %1 s').replace(
        '%1',
        String(Math.ceil(waitTime / 1000)),
      );
    }

    const shown = this.sectionProperties.shown;
    if (
      shown &&
      shown.statusText === statusText &&
      shown.hideTakeOver === hideTakeOver &&
      shown.countdownText === countdownText
    )
      return;
    this.sectionProperties.shown = { statusText, hideTakeOver, countdownText };

    const properties = this.sectionProperties;
    app.layoutingService.appendLayoutingTask(() => {
      properties.status.textContent = statusText;
      properties.takeOverButton.hidden = hideTakeOver;
      properties.wait.hidden = countdownText === null;
      if (countdownText !== null)
        properties.waitText.textContent = countdownText;
    });
  }

  private onTick(now: number) {
    const requestTime = this.sectionProperties.requestTime;
    if (
      requestTime !== null &&
      now - requestTime >= ShapeLockLabelSection.approvalTime
    ) {
      // The other user did not answer, so this user gets the text.
      ShapeLockLabelSection.closeMessage();
      this.takeOverNow(false);
    }
    this.render(now);
  }

  private onTakeOverClick() {
    const phase = this.getPhase(Date.now());
    if (phase === 'active') this.askForText();
    else if (phase === 'inactive') this.takeOverNow(true);
  }

  // The other user gets a request with a countdown, and can reject it.
  private askForText() {
    const now = Date.now();
    this.sectionProperties.requestTime = now;
    ShapeLockLabelSection.fillProgress(
      this.sectionProperties.waitBar,
      ShapeLockLabelSection.approvalTime,
    );
    app.map.sendUnoCommand('.uno:RequestTextEdit', {
      ViewId: { type: 'long', value: this.sectionProperties.viewId },
      ObjectId: { type: 'string', value: this.sectionProperties.objectId },
    });
    ShapeLockLabelSection.showMessage(
      _('%1 needs to approve. After %2 seconds you can edit automatically.')
        .replace('%1', this.getName())
        .replace('%2', String(ShapeLockLabelSection.approvalTime / 1000)),
      ShapeLockLabelSection.approvalTime,
    );
    this.render(now);
  }

  // The other user writes the text into the shape and leaves it, then this user edits it. After a
  // take over without a request, this user can give the text back for a short time.
  private takeOverNow(canGiveBack: boolean) {
    const viewId = this.sectionProperties.viewId;
    const objectId = this.sectionProperties.objectId;
    this.sectionProperties.requestTime = null;
    this.sectionProperties.takeOverTime = Date.now();
    app.map.sendUnoCommand('.uno:TakeOverTextEdit', {
      ViewId: { type: 'long', value: viewId },
      ObjectId: { type: 'string', value: objectId },
    });
    if (canGiveBack) ShapeLockCalloutSection.showGiveBack(viewId, objectId);
    app.map.focus();
  }

  // This user tried to edit the text of the shape. Each key typed into the shape is one more try.
  private onRefused() {
    // The message about the request still shows, or the take over is on its way. A take over that
    // did not arrive in time was refused, so a new try sends another one.
    const now = Date.now();
    const takeOverTime = this.sectionProperties.takeOverTime;
    if (
      this.sectionProperties.requestTime !== null ||
      (takeOverTime !== null &&
        now - takeOverTime < ShapeLockLabelSection.approvalMargin) ||
      !app.map.isEditMode()
    )
      return;

    const phase = this.getPhase(now);
    if (phase === 'editing')
      ShapeLockLabelSection.showMessage(
        _("You can't edit right now, %1 is actively editing.").replace(
          '%1',
          this.getName(),
        ),
      );
    else if (phase === 'active')
      ShapeLockLabelSection.showMessage(
        _('%1 is active. Use Take over in the label.').replace(
          '%1',
          this.getName(),
        ),
      );
    else this.takeOverNow(true);
  }

  private onRejected() {
    if (this.sectionProperties.requestTime === null) return;

    this.sectionProperties.requestTime = null;
    ShapeLockLabelSection.showMessage(
      _('%1 rejected your request.').replace('%1', this.getName()),
    );
    this.render(Date.now());
  }

  // The other user typed or moved the text cursor.
  private onInput(now: number) {
    this.sectionProperties.lastInput = now;
    if (this.sectionProperties.requestTime !== null) {
      this.sectionProperties.requestTime = null;
      ShapeLockLabelSection.showMessage(
        _('%1 started typing again, so nothing changes.').replace(
          '%1',
          this.getName(),
        ),
      );
    }
    this.render(now);
  }

  // Follow the colored border around the shape. Before the border arrives, or when it is on another
  // part, the label follows the area of the text instead.
  private updatePosition() {
    const border = OtherViewGraphicSelectionSection.getViewSection(
      this.sectionProperties.viewId,
    );
    let x: number, y: number, width: number, height: number;

    if (
      border &&
      border.size[0] > 0 &&
      border.size[1] > 0 &&
      border.sectionProperties.part === this.sectionProperties.part
    ) {
      [x, y] = border.position;
      [width, height] = border.size;
    } else {
      const rectangle = this.sectionProperties.lockRectangle;
      [x, y, width, height] = [
        rectangle.pX1,
        rectangle.pY1,
        rectangle.pWidth,
        rectangle.pHeight,
      ];
    }

    // The section covers the shape, so the label shows as long as any of the shape does.
    this.size = [width, height];
    this.setPosition(x, y);
    this.setShowSection(this.checkMyVisibility());
  }

  // The name of the user of the given view.
  public static getName(viewId: number): string {
    return app.map.getViewName(viewId) || '';
  }

  // The picture of the user of the given view, or the initials of the name when the user has no
  // picture. The ring around it has the color of the user.
  public static createAvatar(viewId: number): HTMLElement {
    const username = ShapeLockLabelSection.getName(viewId);
    const avatar = (
      <span class="shape-lock-avatar" aria-hidden="true" title={username} />
    ) as HTMLElement;
    avatar.style.borderColor = app.LOUtil.rgbToHex(
      app.LOUtil.getViewIdColor(viewId),
    );

    const viewInfo = app.map._viewInfo[viewId];
    if (viewInfo && viewInfo.userextrainfo && viewInfo.userextrainfo.avatar) {
      const image = (<img alt="" />) as HTMLImageElement;
      app.LOUtil.setUserImage(image, app.map, viewId);
      avatar.appendChild(image);
    } else {
      const words = username.split(/\s+/).filter((word) => word.length > 0);
      const initials = words
        .filter((word, index) => index === 0 || index === words.length - 1)
        .map((word) => Array.from(word)[0])
        .join('');
      avatar.textContent = initials.toUpperCase();
    }
    return avatar;
  }

  // The bar fills up from empty over the given time, in milliseconds. It grows by a transform, so
  // the page does no layout while it fills.
  public static fillProgress(bar: HTMLElement, duration: number) {
    bar.getAnimations().forEach((animation) => animation.cancel());
    bar.animate([{ transform: 'scaleX(0)' }, { transform: 'scaleX(1)' }], {
      duration: duration,
      easing: 'linear',
      fill: 'forwards',
    });
  }

  // The snackbar shows one message after another, so a message of this label that still shows goes
  // away first. When it is the same message, it stays as it is. A timeout of -1 keeps the message
  // until the user closes it with Dismiss.
  public static showMessage(
    text: string,
    timeout: number = ShapeLockLabelSection.messageTime,
  ) {
    const uiManager = app.map.uiManager;
    if (
      text === ShapeLockLabelSection.messageText &&
      uiManager.hasSnackbar(text)
    )
      return;

    ShapeLockLabelSection.closeMessage();
    ShapeLockLabelSection.messageText = text;
    uiManager.showSnackbar(text, null, null, timeout, false, timeout < 0);
  }

  public static closeMessage() {
    if (ShapeLockLabelSection.messageText !== null)
      app.map.uiManager.closeSnackbarWithLabel(
        ShapeLockLabelSection.messageText,
      );
    ShapeLockLabelSection.messageText = null;
  }

  private static tick() {
    const now = Date.now();
    ShapeLockLabelSection.sections.forEach((section) => section.onTick(now));
  }

  // Keep the shape of the given view locked, or unlock it when rectangle is null. The rectangle is
  // the area of the text in twips, part is the part identifier of the part that holds the shape,
  // and objectId is the unique id of the shape. existing is true when the shape was already locked
  // as this user joined, so how long the other user has not typed is not known.
  public static setLock(
    viewId: number,
    rectangle: cool.SimpleRectangle | null,
    part: string,
    mode: number,
    objectId: string = '',
    existing: boolean = false,
  ) {
    let section = ShapeLockLabelSection.sections.get(viewId);

    if (!rectangle) {
      if (section) {
        // The other user left the text, so a request for it has no answer.
        if (section.sectionProperties.requestTime !== null)
          ShapeLockLabelSection.closeMessage();
        ShapeLockLabelSection.sections.delete(viewId);
        app.sectionContainer.removeSection(section.name);
        app.sectionContainer.requestReDraw();
      }
      if (ShapeLockLabelSection.sections.size === 0) {
        app.timerRegistry.clearInterval(ShapeLockLabelSection.tickTimer);
        ShapeLockLabelSection.tickTimer = null;
      }
      ShapeLockLabelSection.onUnlock(viewId);
      return;
    }

    if (!section) {
      section = new ShapeLockLabelSection(viewId);
      // A user who held the shape before this user joined counts as active, so Take over asks
      // that user first.
      if (existing)
        section.sectionProperties.lastInput =
          Date.now() - ShapeLockLabelSection.activeAfter;
      app.sectionContainer.addSection(section);
      ShapeLockLabelSection.sections.set(viewId, section);
      if (!ShapeLockLabelSection.tickTimer)
        ShapeLockLabelSection.tickTimer = app.timerRegistry.setInterval(
          'shapelocklabel',
          ShapeLockLabelSection.tick,
          250,
        );
    }

    section.sectionProperties.lockRectangle = rectangle;
    section.sectionProperties.objectId = objectId;
    section.sectionProperties.part = part;
    section.sectionProperties.mode = mode;
    section.updatePosition();
    section.render(Date.now());
    app.sectionContainer.requestReDraw();
  }

  // A view that took the text over from this user left it. This user learns about the changes of
  // that view once that view can no longer give the text back. This user may have been away, so the
  // message stays until this user closes it.
  private static onUnlock(viewId: number) {
    const takeOver = ShapeLockLabelSection.takeOvers.get(viewId);
    if (!takeOver || takeOver.canGiveBack) return;

    ShapeLockLabelSection.takeOvers.delete(viewId);
    if (takeOver.hasInput)
      ShapeLockLabelSection.showMessage(
        _('%1 made changes.').replace(
          '%1',
          ShapeLockLabelSection.getName(viewId),
        ),
        -1,
      );
  }

  // The given view took the text that this user edited. Without a request before, that view can
  // give the text back for a short time, so this user learns about it after that time. A view that
  // typed into the text and left it by then is told about as one that made changes. This user may
  // have been away, so the message stays until this user closes it.
  private static onTakenOver(viewId: number) {
    const wasAsked = ShapeLockCalloutSection.takeRequestOf(viewId);
    ShapeLockCalloutSection.close();

    const takeOver: ShapeLockTakeOver = {
      canGiveBack: !wasAsked,
      hasInput: false,
    };
    ShapeLockLabelSection.takeOvers.set(viewId, takeOver);

    const tellTakeOver = () => {
      // The other view gave the text back.
      if (ShapeLockLabelSection.takeOvers.get(viewId) !== takeOver) return;

      takeOver.canGiveBack = false;
      const isEditing = ShapeLockLabelSection.sections.has(viewId);
      if (isEditing || !takeOver.hasInput)
        ShapeLockLabelSection.showMessage(
          _('%1 has taken over editing. Your changes were saved.').replace(
            '%1',
            ShapeLockLabelSection.getName(viewId),
          ),
          -1,
        );
      if (!isEditing) ShapeLockLabelSection.onUnlock(viewId);
    };
    if (wasAsked) tellTakeOver();
    else
      app.timerRegistry.setTimeout(
        'shapelocktakeover',
        tellTakeOver,
        ShapeLockLabelSection.undoTime,
      );
  }

  // A message from the engine about the text of a shape that this user and the given view both
  // want. The reasons are those of the .uno:TextEditLocked state change.
  public static onTextEditLocked(viewId: number, reason: string) {
    const section = ShapeLockLabelSection.sections.get(viewId);
    switch (reason) {
      case 'refused':
        if (section) section.onRefused();
        break;
      case 'rejected':
        if (section) section.onRejected();
        break;
      case 'takenover':
        ShapeLockLabelSection.onTakenOver(viewId);
        break;
      case 'requested':
        ShapeLockCalloutSection.showRequest(viewId);
        break;
      case 'returned':
        // The other view gave the text back before this user was told about the take over.
        ShapeLockLabelSection.takeOvers.delete(viewId);
        break;
    }
  }

  // The text cursor of the given view is at the given place, in twips. A cursor that moved means
  // that the user of that view typed or moved it.
  public static onViewCursor(
    viewId: number,
    rectangle: Array<number> | undefined,
  ) {
    if (
      !rectangle ||
      rectangle.length !== 4 ||
      rectangle.some((value) => isNaN(value))
    )
      return;

    const place = rectangle.join(',');
    const lastPlace = ShapeLockLabelSection.cursors.get(viewId);
    ShapeLockLabelSection.cursors.set(viewId, place);
    if (lastPlace === undefined || lastPlace === place) return;

    const takeOver = ShapeLockLabelSection.takeOvers.get(viewId);
    if (takeOver) takeOver.hasInput = true;

    const section = ShapeLockLabelSection.sections.get(viewId);
    if (section) section.onInput(Date.now());
  }

  // Move the label of the given view after its border moved.
  public static update(viewId: number) {
    const section = ShapeLockLabelSection.sections.get(viewId);
    if (section) section.updatePosition();
  }

  // Show only the labels on the part and in the mode shown now.
  public static updateVisibilities() {
    ShapeLockLabelSection.sections.forEach((section) => {
      section.setShowSection(section.checkMyVisibility());
    });
  }

  public static removeView(viewId: number) {
    ShapeLockLabelSection.takeOvers.delete(viewId);
    ShapeLockLabelSection.cursors.delete(viewId);
    ShapeLockLabelSection.setLock(viewId, null, '', 0);
    ShapeLockCalloutSection.closeIfFrom(viewId);
  }
}

app.definitions.shapeLockLabelSection = ShapeLockLabelSection;
