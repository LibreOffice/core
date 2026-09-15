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
 * SlideLinkToolbar - the small toolbar over the slide on show when that slide
 * is linked to another file. It says that the slide is linked and offers what
 * can be done about this one slide: take the link off, and read the slide
 * again from the file it came from.
 *
 * The toolbar follows the slide on show and acts on that slide alone. Folded,
 * it names the link and offers an update when the file has moved on since the
 * slide was read from it. Unfolded, it offers every action. Taking the link
 * off and reading the slide again change the document, so a view that cannot
 * edit is offered neither.
 */

/* global app _ */

class SlideLinkToolbar {
  private map: any;
  // The element the toolbar is drawn in, or null when the slide on show is
  // linked to nothing.
  private element: HTMLElement | null = null;
  // Whether every action is on show, or the link alone with an update when
  // one is due. The user's choice, kept while the document is open.
  private expanded: boolean = false;

  constructor(map: any) {
    this.map = map;

    map.on('docloaded', this.refresh, this);
    map.on('setpart', this.refresh, this);
    map.on('updateparts', this.refresh, this);
    app.events.on('slidelink:changed', this.refresh.bind(this));
    app.events.on('updatepermission', this.refresh.bind(this));
  }

  public isExpanded(): boolean {
    return this.expanded;
  }

  public setExpanded(expanded: boolean): void {
    this.expanded = expanded;
    this.refresh();
  }

  // The identifier of the slide on show when that slide is linked to another
  // file, or an empty string for a slide that is linked to nothing.
  private linkedPart(): string {
    const links = this.map.slideLinks;
    if (!links || !this.map.isPresentationOrDrawing()) return '';
    const part = links.currentPart();
    return part && links.getPageLink(part) !== null ? part : '';
  }

  // Draws the toolbar for the slide on show, or takes it away when that slide
  // is linked to nothing. The toolbar is small, so it is drawn afresh each
  // time rather than kept in step piece by piece.
  public refresh(): void {
    // A toolbar that held the focus hands it to the one drawn in its place.
    const focused =
      this.element !== null && this.element.contains(document.activeElement);
    if (this.element) {
      this.element.remove();
      this.element = null;
    }

    const part = this.linkedPart();
    if (!part) return;

    const container = document.getElementById('document-container');
    if (!container) return;

    this.element = this.render(part);
    container.appendChild(this.element);
    if (focused)
      (
        this.element.querySelector(
          '.slide-link-toolbar-expander',
        ) as HTMLElement
      ).focus();
  }

  private render(part: string): HTMLElement {
    const links = this.map.slideLinks;
    const link = links.getPageLink(part);
    const editable = this.map.isEditMode();
    const outdated = links.isPageOutdated(part);
    const readable = links.isPageUpdatable(part);
    const stateLabel = this.sourceStateLabel(links.getPageSourceState(part));

    // The label describes the link the way the slide's thumbnail does.
    const description = _('Linked to {0} in {1}')
      .replace('{0}', () => link.name)
      .replace('{1}', () => link.source);

    const element = (
      <div
        id="slide-link-toolbar"
        class={'slide-link-toolbar' + (this.expanded ? ' expanded' : '')}
        role="toolbar"
        aria-label={description}
      >
        <span class="slide-link-toolbar-badge" aria-hidden="true"></span>
        <span class="slide-link-toolbar-title" title={description}>
          {_('Linked')}
        </span>
        {stateLabel ? (
          <span class="slide-link-toolbar-state">{stateLabel}</span>
        ) : null}
        <button
          type="button"
          class="slide-link-toolbar-expander"
          aria-expanded={this.expanded ? 'true' : 'false'}
          aria-label={
            this.expanded
              ? _('Hide the actions for this linked slide')
              : _('Show the actions for this linked slide')
          }
          onClick={() => this.setExpanded(!this.expanded)}
        ></button>
        {this.expanded && editable ? (
          <button
            type="button"
            class="button slide-link-toolbar-action unlink"
            onClick={() => app.dispatcher.dispatch('unlinkslide')}
          >
            {_('Unlink')}
          </button>
        ) : null}
        {(this.expanded || outdated) && editable ? (
          <button
            type="button"
            class="button slide-link-toolbar-action update"
            disabled={!readable}
            onClick={() => app.dispatcher.dispatch('updatelinkedslide')}
          >
            {_('Update')}
          </button>
        ) : null}
      </div>
    );

    return element;
  }

  // What the toolbar says about a source that cannot be read, next to the
  // link. Nothing for a source that can.
  private sourceStateLabel(state: string): string {
    switch (state) {
      case '':
        return _('Not available');
      case 'noaccess':
        return _('No access');
      case 'missing':
        return _('Missing');
    }
    return '';
  }
}
