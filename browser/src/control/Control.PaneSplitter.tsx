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
 * PaneSplitter - the draggable divider between the two maximized slide
 * grids, the navigation sidebar's and the slide import pane's. The user
 * drags it to trade width between the grids; the width is stored as a CSS
 * custom property on #main-document-content so the layout picks it up
 * without a reflow of the whole page. The docked side-by-side panels have
 * fixed widths, so the divider only shows while the panels are maximized
 * over the document area.
 *
 * The grids start at an even split, each keeps a minimum width, and a
 * double click returns to the even split. A width lasts only for the
 * session: every document opens at the default.
 */

/* global _ */

class PaneSplitter {
  private map: any;
  private mainContent: HTMLElement;
  private handle: HTMLElement;
  private pointerId: number = -1;
  // Pointer x, the controlled width and its upper limit, all captured when
  // the drag began. The limit depends only on the container, which does not
  // change during a drag, so measuring it once keeps the pointermove
  // handler free of layout reads.
  private startX: number = 0;
  private startWidth: number = 0;
  private startUpperLimit: number = 0;

  // The width of the maximized navigation grid, in pixels; the import grid
  // fills the rest of the document area. Unset, the grids split the area
  // evenly.
  private static readonly PROPERTY_NAME: string = '--expanded-navigation-width';
  // Each grid keeps at least this width, in pixels.
  private static readonly MIN_WIDTH: number = 240;
  // How far one arrow-key press moves the divider, in pixels.
  private static readonly KEY_STEP: number = 16;

  constructor(map: any) {
    this.map = map;
    this.mainContent = document.getElementById('main-document-content');
    if (!this.mainContent) return;

    this.handle = this.createHandle();
    this.mainContent.appendChild(this.handle);
    this.updateAria();
  }

  // The divider is a vertical bar, so left and right resize it. A screen
  // reader announces it as a separator carrying the current width.
  private createHandle(): HTMLElement {
    return (
      <div
        class="pane-splitter"
        role="separator"
        aria-orientation="vertical"
        tabindex="0"
        aria-label={_('Resize panels')}
        onPointerDown={(e: PointerEvent) => this.onPointerDown(e)}
        onPointerMove={(e: PointerEvent) => this.onPointerMove(e)}
        onPointerUp={(e: PointerEvent) => this.onPointerUp(e)}
        onPointerCancel={(e: PointerEvent) => this.onPointerUp(e)}
        onDblClick={() => this.onDoubleClick()}
        onKeyDown={(e: KeyboardEvent) => this.onKeyDown(e)}
      ></div>
    ) as HTMLElement;
  }

  // The panels are maximized, so the divider is on show and its moves
  // apply.
  private isActive(): boolean {
    return (
      this.map.paneExpander && this.map.paneExpander.getMode() === 'expanded'
    );
  }

  // The width the divider returns to on a double click, and the width it
  // reports before the first drag: an even split of the container.
  private defaultWidth(): number {
    return Math.round(this.mainContent.getBoundingClientRect().width / 2);
  }

  private currentWidth(): number {
    const value = this.mainContent.style.getPropertyValue(
      PaneSplitter.PROPERTY_NAME,
    );
    const parsed = parseFloat(value);
    return isNaN(parsed) ? this.defaultWidth() : parsed;
  }

  // The widest the navigation grid may get: the container less the minimum
  // the import grid keeps on the far side of the divider.
  private upperLimit(): number {
    const available = this.mainContent.getBoundingClientRect().width;
    return Math.max(PaneSplitter.MIN_WIDTH, available - PaneSplitter.MIN_WIDTH);
  }

  // Update the layout to a new width, clamped between the grids' minimum
  // and the given upper limit.
  private applyWidth(px: number, upper: number): void {
    const clamped = Math.round(
      Math.max(PaneSplitter.MIN_WIDTH, Math.min(upper, px)),
    );
    this.mainContent.style.setProperty(
      PaneSplitter.PROPERTY_NAME,
      clamped + 'px',
    );
    this.setAriaValues(PaneSplitter.MIN_WIDTH, upper, clamped);
  }

  private onPointerDown(e: PointerEvent): void {
    // Only the primary button of the primary pointer resizes; a secondary
    // button (right or middle click) or a second touch point leaves the
    // boundary alone.
    if (e.button > 0 || e.isPrimary === false) return;
    this.startX = e.clientX;
    this.startWidth = this.currentWidth();
    this.startUpperLimit = this.upperLimit();
    this.pointerId = e.pointerId;
    this.handle.setPointerCapture(e.pointerId);
    this.handle.classList.add('dragging');
    e.preventDefault();
  }

  private onPointerMove(e: PointerEvent): void {
    if (this.pointerId !== e.pointerId) return;
    // Escape can return to the normal layout mid-drag; the moves that
    // arrive after that no longer have a boundary to place.
    if (!this.isActive()) return;
    const rtl = document.documentElement.dir === 'rtl';
    const inlineDelta = (rtl ? -1 : 1) * (e.clientX - this.startX);
    this.applyWidth(this.startWidth + inlineDelta, this.startUpperLimit);
  }

  private onPointerUp(e: PointerEvent): void {
    if (this.pointerId !== e.pointerId) return;
    if (this.handle.hasPointerCapture(e.pointerId))
      this.handle.releasePointerCapture(e.pointerId);
    this.pointerId = -1;
    this.handle.classList.remove('dragging');
  }

  // A double click puts the grids back to the even split.
  private onDoubleClick(): void {
    this.applyWidth(this.defaultWidth(), this.upperLimit());
  }

  private onKeyDown(e: KeyboardEvent): void {
    const rtl = document.documentElement.dir === 'rtl';
    let inlineDelta = 0;
    if (e.key === 'ArrowRight') inlineDelta = rtl ? -1 : 1;
    else if (e.key === 'ArrowLeft') inlineDelta = rtl ? 1 : -1;
    else if (e.key === 'Home') {
      this.applyWidth(PaneSplitter.MIN_WIDTH, this.upperLimit());
      e.preventDefault();
      return;
    } else if (e.key === 'End') {
      const upper = this.upperLimit();
      this.applyWidth(upper, upper);
      e.preventDefault();
      return;
    } else return;

    this.applyWidth(
      this.currentWidth() + inlineDelta * PaneSplitter.KEY_STEP,
      this.upperLimit(),
    );
    e.preventDefault();
  }

  private setAriaValues(min: number, max: number, now: number): void {
    this.handle.setAttribute('aria-valuemin', String(min));
    this.handle.setAttribute('aria-valuemax', String(Math.round(max)));
    this.handle.setAttribute('aria-valuenow', String(Math.round(now)));
  }

  // Report the width the divider controls, so its position is spoken
  // correctly when the maximized layout brings it on show.
  private updateAria(): void {
    if (!this.handle) return;
    this.setAriaValues(
      PaneSplitter.MIN_WIDTH,
      this.upperLimit(),
      this.currentWidth(),
    );
  }

  // The expand mode changed; refresh the spoken position to match the
  // boundary the divider now sits on.
  public onModeChange(): void {
    this.updateAria();
  }
}
