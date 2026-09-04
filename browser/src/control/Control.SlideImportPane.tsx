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
 * SlideImportPane - the slide import pane docked beside the navigation
 * sidebar in a presentation. The user picks another presentation file,
 * previews its slides as thumbnails, selects some and inserts them into
 * the open document. All state lives in SlideImportSession; this class
 * only renders it and translates user input into session calls.
 */

/* global app _ */

class SlideImportPane {
  private map: any;
  private session: SlideImportSession;
  private wrapper: HTMLElement;
  private panel: HTMLElement;
  private visible: boolean = false;
  // Index of the slide frame that holds the roving tab stop.
  private focusIndex: number = 0;
  // Anchor slide for shift-click and shift-arrow range selection.
  private anchorIndex: number = 0;
  // Names of sections collapsed by the user, like the slide navigator keeps
  // them. Cleared when a new file is opened.
  private collapsedSections: Set<string> = new Set();
  // Index of the slide whose corner checkbox the primary button is pressed
  // on, or -1. The click handler reads it so a press that starts on the
  // checkbox toggles the slide even when the release lands beside it.
  private pressedCheckboxIndex: number = -1;

  // Whether the pages this pane asked a source to write are still to come.
  private awaitingExport: boolean = false;
  // The connected remote document whose slides the pane is showing, named by
  // its WOPISrc, or empty when none is open. The remote's structure and
  // thumbnails below belong to it.
  private selectedRemote: string = '';
  private remoteDocWidth: number = 0;
  private remoteDocHeight: number = 0;
  // The stable part number of each slide, in slide order. A presentation
  // names its pages by these numbers, not by their position, so a tile
  // request has to carry the number of the slide it wants.
  private remoteSlideParts: number[] = [];

  constructor(map: any) {
    this.map = map;
    this.session = new SlideImportSession(map);
    this.wrapper = document.getElementById(
      'slide-import-dock-wrapper',
    ) as HTMLElement;
    this.panel = document.getElementById('slide-import-panel') as HTMLElement;

    app.events.on('slideimport:statechange', this.onSessionChanged.bind(this));
    app.events.on('slideimport:error', this.onSessionChanged.bind(this));
    app.events.on('slideimport:selection', this.onSelectionChanged.bind(this));
    app.events.on('slideimport:thumbnail', this.onThumbnail.bind(this));
    app.events.on('slideimport:inserted', this.onInserted.bind(this));
    app.events.on('slideimport:exportwanted', this.onExportWanted.bind(this));
    app.events.on('updatepermission', this.onUpdatePermission.bind(this));
    map.on('docloaded', this.onDocLoaded, this);
    map.on('relateddocuments', this.onRelatedDocuments, this);
    map.on('remotedoccommandresult', this.onRemoteResult, this);
  }

  public isVisible(): boolean {
    return this.visible;
  }

  public open(): void {
    if (this.visible || !this.isAvailable()) return;
    this.visible = true;
    this.wrapper.classList.add('visible');
    // A pane opened while the panels are maximized joins the grid layout
    // directly.
    this.setGridMode(
      this.map.paneExpander && this.map.paneExpander.getMode() === 'expanded',
    );
    this.render();
  }

  public close(): void {
    if (!this.visible) return;
    this.visible = false;
    if (this.map.paneExpander)
      this.map.paneExpander.onPanelClosing('slide-import-dock-wrapper');
    this.wrapper.classList.remove('visible');
    this.setGridMode(false);
    this.session.close();
    this.selectedRemote = '';
    this.panel.replaceChildren();
  }

  // While the panels are maximized the thumbnails flow into a grid. The
  // class is the one the slide navigator's wrapper carries in that state,
  // so one set of grid rules lays out both panels. Only an open pane takes
  // it: the class forces the wrapper on show, so it would reveal a closed
  // pane.
  public setGridMode(enabled: boolean): void {
    this.wrapper.classList.toggle(
      'parts-preview-grid',
      enabled && this.visible,
    );
  }

  private toggleExpand(): void {
    if (this.map.paneExpander) this.map.paneExpander.toggle();
  }

  public toggle(): void {
    if (this.visible) this.close();
    else this.open();
  }

  private isAvailable(): boolean {
    return (
      this.map.getDocType() === 'presentation' &&
      !window.mode.isSmallScreenDevice() &&
      this.map.isEditMode()
    );
  }

  private onUpdatePermission(e: any): void {
    if (e.detail.perm !== 'edit') this.close();
  }

  private onDocLoaded(e: any): void {
    if (e.status === false) this.close();
  }

  private onSessionChanged(): void {
    if (this.visible) this.render();
  }

  private onRelatedDocuments(): void {
    // Drop the open remote view when its link is no longer connected.
    if (this.selectedRemote) {
      const open = SlideImportSession.findRelatedDocument(this.selectedRemote);
      if (!open || open.state !== 'connected') {
        this.selectedRemote = '';
        this.session.close();
      }
    }

    if (this.visible) this.render();
  }

  // The file name part of a related document's WOPISrc.
  private relatedDocumentName(wopiSrc: string): string {
    return SlideImportSession.relatedDocumentName(wopiSrc);
  }

  // Asks the server to open a live link to the related document; the state
  // in the next relateddocuments: message follows the subscription.
  private subscribeRelatedDocument(wopiSrc: string): void {
    SlideImportSession.subscribeRelatedDocument(wopiSrc);
  }

  // The "Browse files for import" button asks the integration to open its own
  // file chooser, filtered to presentations, and to add the picked file as a
  // related document. The integration registers it with a POST to
  // /cool/relateddocument using the one-time token below. The added file then
  // arrives in the related documents list, from where its slides are imported.
  private browseForImport(): void {
    if (!app.relatedDocumentToken) return;

    app.map.fire('postMessage', {
      msgId: 'UI_AddRelatedDocument',
      args: {
        Nonce: app.relatedDocumentToken,
        WOPISrc: window.wopiSrc,
        Endpoint:
          window.makeHttpUrl('/cool/relateddocument') +
          '?WOPISrc=' +
          encodeURIComponent(window.wopiSrc),
        mimeTypeFilter: app.LOUtil.presentationMimeFilter,
      },
    });
  }

  // Sends a read-only client command to a subscribed remote document. Its
  // reply arrives as a remotedoccommandresult map event carrying the same
  // wopiSrc.
  private sendRemoteCommand(wopiSrc: string, inner: string): void {
    SlideImportSession.sendRemoteCommand(wopiSrc, inner);
  }

  // Opens a connected remote document in the pane and asks it for its
  // structure. Thumbnails follow once the presentation info gives the slide
  // count and page size.
  private openRemoteDocument(wopiSrc: string): void {
    // Empty the list so it does not show the previous file's or remote's slides.
    this.session.close();
    this.selectedRemote = wopiSrc;
    this.remoteDocWidth = 0;
    this.remoteDocHeight = 0;
    this.remoteSlideParts = [];
    // A newly opened remote document starts with every section expanded and
    // the list focus at its first slide, like a freshly opened file.
    this.collapsedSections.clear();
    this.focusIndex = 0;
    this.anchorIndex = 0;

    // The pages of a link insert record the document they came from, as the user knows it.
    this.session.setSource(this.relatedDocumentName(wopiSrc));

    this.sendRemoteCommand(wopiSrc, 'getpresentationinfo');
    this.sendRemoteCommand(wopiSrc, 'getslidesections');
    if (this.visible) this.render();
  }

  // A reply from the remote document currently open in the pane.
  private onRemoteResult(e: {
    wopiSrc: string;
    textMsg: string;
    imgBytes?: Uint8Array;
    imgIndex?: number;
  }): void {
    if (!this.selectedRemote || e.wopiSrc !== this.selectedRemote) return;

    const textMsg = e.textMsg || '';
    if (textMsg.startsWith('presentationinfo:')) {
      this.onRemotePresentationInfo(
        textMsg.substring('presentationinfo:'.length),
      );
    } else if (textMsg.startsWith('slidesections:')) {
      this.onRemoteSlideSections(textMsg.substring('slidesections:'.length));
    } else if (textMsg.startsWith('tile:')) {
      this.onRemoteThumbnail(textMsg, e.imgBytes, e.imgIndex ?? 0);
    } else if (textMsg.startsWith('exportslides:')) {
      this.onRemoteExport(textMsg.substring('exportslides:'.length));
    }
  }

  // The slides a user chose are written out by the source as a presentation of their own. The
  // server stages it in this document's jail and names it here, and the insert takes that file.
  private onExportWanted(): void {
    const insert = this.session.getPendingInsert();
    if (!this.selectedRemote || !insert) return;

    const parts = insert.slides
      .map((index: number) => this.remoteSlideParts[index])
      .filter((part: number) => part > 0);
    if (!parts.length) return;

    // Update Linked Slides asks the same document for its pages, so only the export this
    // pane asked for is taken as the file an insert reads.
    this.awaitingExport = true;
    this.sendRemoteCommand(
      this.selectedRemote,
      'exportslides slides=' + parts.join(','),
    );
  }

  private onRemoteExport(json: string): void {
    if (!this.awaitingExport) return;
    this.awaitingExport = false;

    const insert = this.session.getPendingInsert();
    if (!insert) return;

    const stagedName = SlideImportSession.stagedExportName(json);
    if (!stagedName) {
      this.session.exportFailed();
      return;
    }

    this.session.insertStaged(stagedName, insert.at);
  }

  private onRemotePresentationInfo(json: string): void {
    try {
      const info = JSON.parse(json);
      this.remoteDocWidth = info.docWidth || 0;
      this.remoteDocHeight = info.docHeight || 0;
      const slides = Array.isArray(info.slides) ? info.slides : [];
      this.remoteSlideParts = slides.map((s: any) => s.uniqueID || 0);
      this.session.slides = slides.map((s: any, index: number) => ({
        index: index,
        name: '',
        guid: s.guid || '',
      }));
      this.session.slideCount = this.remoteSlideParts.length;
      this.session.size = {
        width: this.remoteDocWidth,
        height: this.remoteDocHeight,
      };
    } catch (err) {
      app.console.error('Bad remote presentationinfo: ' + err);
      return;
    }

    // The slides are on show, so they can be chosen and inserted.
    this.session.slidesShown();

    this.requestRemoteThumbnails();
    if (this.visible) this.render();
  }

  private onRemoteSlideSections(json: string): void {
    try {
      const sections = JSON.parse(json);
      this.session.sections = Array.isArray(sections)
        ? sections.map((s: any) => ({
            name: s.name || '',
            startIndex: s.startIndex || 0,
            slideCount: s.slideCount || 0,
          }))
        : [];
    } catch (err) {
      app.console.error('Bad remote slidesections: ' + err);
      return;
    }

    if (this.visible) this.render();
  }

  // Asks the remote for a PNG preview of every slide, fitted to the pane
  // width, keyed by part index so each reply lands in its slot.
  private requestRemoteThumbnails(): void {
    if (
      !this.selectedRemote ||
      this.remoteSlideParts.length <= 0 ||
      this.remoteDocWidth <= 0 ||
      this.remoteDocHeight <= 0
    )
      return;

    const targetWidth = 160;
    const targetHeight = Math.max(
      1,
      Math.round((targetWidth * this.remoteDocHeight) / this.remoteDocWidth),
    );
    const scale = app.roundedDpiScale || 1;

    for (let slot = 0; slot < this.remoteSlideParts.length; ++slot) {
      // The request names the slide by its stable part number, which the
      // remote resolves to the page it paints. The slot index rides along as
      // the preview id: it is a non-negative integer, so the remote treats
      // the request as a preview and echoes the id back, which lands the
      // reply in the right thumbnail slot.
      const partNumber = this.remoteSlideParts[slot];
      if (!(partNumber > 0)) continue;
      this.sendRemoteCommand(
        this.selectedRemote,
        'tile nviewid=0 part=' +
          partNumber +
          ' mode=0 width=' +
          Math.round(targetWidth * scale) +
          ' height=' +
          Math.round(targetHeight * scale) +
          ' tileposx=0 tileposy=0 tilewidth=' +
          this.remoteDocWidth +
          ' tileheight=' +
          this.remoteDocHeight +
          ' id=' +
          slot,
      );
    }
  }

  private onRemoteThumbnail(
    textMsg: string,
    imgBytes: Uint8Array | undefined,
    imgIndex: number,
  ): void {
    if (!imgBytes) return;

    // A preview tile echoes its slot index as the integer id. A tile with
    // no id is not one of our thumbnails.
    const id = new ServerCommand(textMsg, this.map).id;
    if (id === undefined || id === '') return;

    const slot = parseInt(id, 10);
    if (isNaN(slot) || slot < 0 || slot >= this.remoteSlideParts.length) return;

    this.session.thumbnails.set(slot, {
      url: app.socket.pngDataUrl(imgBytes.subarray(imgIndex)),
      width: this.remoteDocWidth,
      height: this.remoteDocHeight,
    });

    // The slide frames already exist from the presentation info, so drop the
    // arrived thumbnail into its slot without rebuilding the list.
    this.updateThumbnail(slot);
  }

  private relatedDocumentStateLabel(state: string): string {
    switch (state) {
      case 'noaccess':
        return _('No access');
      case 'subscribed':
        return _('Subscribed');
      case 'connected':
        return _('Connected');
      case 'disconnected':
        return _('Disconnected');
      case 'failed':
        return _('Failed');
      case 'missing':
        return _('Missing');
      default:
        return _('Available');
    }
  }

  // The related documents the server announced for this document, with the
  // state of their subscriptions.
  private renderRelatedDocuments(): HTMLElement | null {
    const documents = app.relatedDocuments || [];
    if (!documents.length) return null;

    return (
      <div class="slide-import-related">
        <div class="slide-import-related-title">{_('Related documents')}</div>
        <ul
          class="slide-import-related-list"
          aria-label={_('Related documents')}
        >
          {documents.map((doc: { wopiSrc: string; state: string }) => (
            <li
              class="slide-import-related-item"
              data-state={doc.state}
              title={this.relatedDocumentName(doc.wopiSrc)}
            >
              <span class="slide-import-related-name">
                {this.relatedDocumentName(doc.wopiSrc)}
              </span>
              {doc.state === 'available' ? (
                <button
                  class="button slide-import-related-subscribe"
                  aria-label={
                    _('Subscribe to') +
                    ' ' +
                    this.relatedDocumentName(doc.wopiSrc)
                  }
                  onClick={() => this.subscribeRelatedDocument(doc.wopiSrc)}
                >
                  {_('Subscribe')}
                </button>
              ) : doc.state === 'connected' ? (
                <button
                  class="button slide-import-related-open"
                  aria-pressed={
                    this.selectedRemote === doc.wopiSrc ? 'true' : 'false'
                  }
                  aria-label={
                    _('Open') + ' ' + this.relatedDocumentName(doc.wopiSrc)
                  }
                  onClick={() => this.openRemoteDocument(doc.wopiSrc)}
                >
                  {_('Open')}
                </button>
              ) : (
                <span class="slide-import-related-state">
                  {this.relatedDocumentStateLabel(doc.state)}
                </span>
              )}
            </li>
          ))}
        </ul>
      </div>
    );
  }

  private onSelectionChanged(): void {
    if (!this.visible) return;
    this.panel
      .querySelectorAll('.slide-import-slide')
      .forEach((option: Element) => {
        const index = parseInt(
          (option as HTMLElement).dataset.index as string,
          10,
        );
        const selected = this.session.selection.has(index);
        option.setAttribute('aria-selected', selected ? 'true' : 'false');
        const img = option.querySelector('.preview-img');
        if (img) img.classList.toggle('preview-img-selectedpart', selected);
        const checkbox = option.querySelector('.slide-import-checkbox');
        if (checkbox) checkbox.classList.toggle('checked', selected);
      });
    this.updateSectionSelection();
    this.updateInsertButton();
  }

  private onThumbnail(e: any): void {
    this.updateThumbnail(e.detail.index);
  }

  // Drops the thumbnail that arrived for a slide into its frame, replacing
  // the skeleton, without rebuilding the list.
  private updateThumbnail(index: number): void {
    if (!this.visible) return;
    const option = this.panel.querySelector(
      '.slide-import-slide[data-index="' + index + '"]',
    );
    const thumbnail = this.session.thumbnails.get(index);
    if (!option || !thumbnail) return;
    const img = option.querySelector('.preview-img') as HTMLImageElement;
    if (!img) return;
    img.src = thumbnail.url;
    img.classList.remove('slide-import-skeleton');
  }

  private onInserted(e: any): void {
    const count = e.detail.count;
    const label =
      count === 1
        ? _('1 slide inserted')
        : _('{0} slides inserted').replace('{0}', String(count));
    this.map.uiManager.showSnackbar(label);
  }

  private slideLabel(index: number): string {
    const slide = this.session.slides[index];
    const name = slide && slide.name ? slide.name : '';
    return name ? name : _('Slide {0}').replace('{0}', String(index + 1));
  }

  private insertButtonLabel(): string {
    const count = this.session.selection.size;
    if (count === 1) return _('Insert 1 slide');
    if (count > 1) return _('Insert {0} slides').replace('{0}', String(count));
    return _('Insert slides');
  }

  private updateInsertButton(): void {
    const button = this.panel.querySelector(
      '.slide-import-insert',
    ) as HTMLButtonElement | null;
    if (!button) return;
    button.textContent = this.insertButtonLabel();
    button.disabled =
      this.session.selection.size === 0 || this.session.state !== 'ready';
  }

  private statusText(): string {
    if (this.session.state === 'opening') return _('Opening file...');
    if (this.session.state === 'inserting') return _('Inserting slides...');
    return '';
  }

  private render(): void {
    const session = this.session;
    // The session holds either the chosen file's slides or, when a remote
    // document is open, the remote's; the file-only controls above the list
    // show only for a local import.
    const list = this.panel.querySelector('.slide-import-list');
    const scrollTop = list ? list.scrollTop : 0;

    if (this.focusIndex >= session.slideCount) this.focusIndex = 0;
    if (this.anchorIndex >= session.slideCount) this.anchorIndex = 0;

    const status = this.statusText();

    // The header carries the slide navigator's header classes, so both
    // panels share one set of header, title and button styles.
    this.panel.replaceChildren(
      <div class="slide-import">
        <div class="navigation-header">
          <h2 class="navigation-title slide-import-title">
            {_('Import slides')}
          </h2>
          <button
            class="navigation-expand-button slide-import-expand"
            aria-label={_('Expand slide import')}
            data-cooltip={_('Expand slide import')}
            aria-pressed={
              this.map.paneExpander &&
              this.map.paneExpander.getMode() === 'expanded'
                ? 'true'
                : 'false'
            }
            onClick={() => this.toggleExpand()}
          >
            <img src={app.LOUtil.getImageURL('lc_fullscreen.svg')} alt="" />
          </button>
          <div class="close-navigation-wrapper">
            <button
              class="close-navigation-button slide-import-close"
              aria-label={_('Close slide import')}
              data-cooltip={_('Close slide import')}
              onClick={() => this.close()}
            />
          </div>
        </div>
        <div class="slide-import-body">
          <button
            class="button slide-import-choose"
            disabled={session.state === 'opening' || !app.relatedDocumentToken}
            onClick={() => this.browseForImport()}
          >
            {_('Browse files for import')}
          </button>
          {session.fileName && (
            <div class="slide-import-filename">{session.fileName}</div>
          )}
          {!session.fileName && !status && !session.error && (
            <div class="slide-import-hint">
              {_('Choose a presentation file to import slides from.')}
            </div>
          )}
          {this.renderRelatedDocuments()}
          <div
            class="slide-import-status"
            role="status"
            aria-live="polite"
            style={{ display: status ? '' : 'none' }}
          >
            {status}
          </div>
          {session.error && (
            <div class="slide-import-error" role="alert">
              <span>{session.error.message}</span>
              {session.error.retry && (
                <button
                  class="button slide-import-retry"
                  onClick={() => session.error.retry()}
                >
                  {_('Retry')}
                </button>
              )}
            </div>
          )}
          {session.slideCount > 0 && (
            <label class="slide-import-keepdesign">
              <input
                type="checkbox"
                checked={session.keepDesign}
                onChange={(e: Event) =>
                  session.setKeepDesign((e.target as HTMLInputElement).checked)
                }
              />
              {_('Keep original design')}
            </label>
          )}
          {session.slideCount > 0 && session.canLink && (
            <label class="slide-import-linksource">
              <input
                type="checkbox"
                checked={session.linkToSource}
                onChange={(e: Event) =>
                  session.setLinkToSource(
                    (e.target as HTMLInputElement).checked,
                  )
                }
              />
              {_('Link to the source file')}
            </label>
          )}
          {session.slideCount > 0 &&
            session.canLink &&
            session.linkToSource && (
              <label class="slide-import-positionnames">
                <input
                  type="checkbox"
                  checked={session.updateByPosition}
                  onChange={(e: Event) =>
                    session.setUpdateByPosition(
                      (e.target as HTMLInputElement).checked,
                    )
                  }
                />
                {_(
                  'Update by slide position for sources without slide identifiers',
                )}
              </label>
            )}
          {session.slideCount > 0 && (
            <div
              class="slide-import-list"
              role="listbox"
              aria-multiselectable="true"
              aria-label={_('Slides in the chosen file')}
              onKeyDown={(e: KeyboardEvent) => this.onListKeyDown(e)}
            >
              {this.renderListItems()}
            </div>
          )}
          {session.slideCount > 0 && (
            <div class="slide-import-footer">
              <button
                class="button button-primary slide-import-insert"
                disabled={
                  session.selection.size === 0 || session.state !== 'ready'
                }
                onClick={() => session.insertSelected()}
              >
                {this.insertButtonLabel()}
              </button>
            </div>
          )}
        </div>
      </div>,
    );

    // The header buttons show their labels as tooltips, like the
    // navigator's header buttons do.
    this.panel
      .querySelectorAll('.navigation-expand-button, .close-navigation-button')
      .forEach((button: Element) =>
        window.L.control.attachTooltipEventListener(button, this.map),
      );

    const newList = this.panel.querySelector('.slide-import-list');
    if (newList) newList.scrollTop = scrollTop;
  }

  // The slides of the chosen file in order, with a header before the first
  // slide of each section, like the slide navigator shows the document's
  // own sections. Slides and sections are both in slide order, so one
  // pointer walks the sections along with the slides; consecutive sections
  // can share a start slide when a section is empty, and each still gets
  // its own header.
  private renderListItems(): HTMLElement[] {
    const session = this.session;
    const items: HTMLElement[] = [];
    let nextSection = 0;
    session.slides.forEach((slide) => {
      while (
        nextSection < session.sections.length &&
        session.sections[nextSection].startIndex <= slide.index
      ) {
        items.push(this.renderSectionHeader(nextSection));
        nextSection++;
      }
      items.push(this.renderSlide(slide.index));
    });
    return items;
  }

  // The section a slide belongs to: the last section starting at or before
  // it. Slides in front of the first section belong to no section.
  private sectionOf(index: number): SlideImportSection | null {
    let result: SlideImportSection | null = null;
    for (const section of this.session.sections) {
      if (section.startIndex > index) break;
      result = section;
    }
    return result;
  }

  private isSlideCollapsed(index: number): boolean {
    const section = this.sectionOf(index);
    return section !== null && this.collapsedSections.has(section.name);
  }

  private isSectionFullySelected(sectionIndex: number): boolean {
    const section = this.session.sections[sectionIndex];
    if (!section || section.slideCount <= 0) return false;
    for (
      let i = section.startIndex;
      i < section.startIndex + section.slideCount;
      i++
    ) {
      if (!this.session.selection.has(i)) return false;
    }
    return true;
  }

  // The header comes from the builder the slide navigator uses too, so both
  // panels show sections with the same markup and controls.
  private renderSectionHeader(sectionIndex: number): HTMLElement {
    const section = this.session.sections[sectionIndex];
    const header = buildSlideSectionHeader(
      section.name,
      sectionIndex,
      this.collapsedSections.has(section.name),
      {
        onToggle: () => this.toggleSectionCollapse(sectionIndex),
        onSelect: () => this.selectSection(sectionIndex),
      },
    );
    if (this.isSectionFullySelected(sectionIndex))
      header.querySelector('.slide-section-name').classList.add('selected');
    return header;
  }

  private toggleSectionCollapse(sectionIndex: number): void {
    const section = this.session.sections[sectionIndex];
    if (!section) return;
    if (this.collapsedSections.has(section.name))
      this.collapsedSections.delete(section.name);
    else this.collapsedSections.add(section.name);
    // A hidden slide cannot hold the list's tab stop; hand it to the
    // first slide still on show.
    if (this.isSlideCollapsed(this.focusIndex)) {
      const firstVisible = this.session.slides
        .map((slide) => slide.index)
        .find((index) => !this.isSlideCollapsed(index));
      if (firstVisible !== undefined) this.focusIndex = firstVisible;
    }
    this.render();
    // The rebuild replaced the pressed chevron; keyboard focus continues on
    // its replacement.
    const toggle = this.panel.querySelector(
      '.slide-section-header[data-section-index="' +
        sectionIndex +
        '"] .slide-section-toggle',
    ) as HTMLElement | null;
    if (toggle) toggle.focus();
  }

  // Clicking a header selects every slide of its section, like the slide
  // navigator does.
  private selectSection(sectionIndex: number): void {
    const section = this.session.sections[sectionIndex];
    if (!section || section.slideCount <= 0) return;
    const start = section.startIndex;
    this.anchorIndex = start;
    this.session.selectRange(start, start + section.slideCount - 1);
    // A collapsed section's slides are hidden and cannot hold the list's
    // tab stop, so it stays where it is.
    if (!this.isSlideCollapsed(start)) this.setFocusIndex(start, false);
  }

  // A section whose slides are all selected shows its name highlighted,
  // like the slide navigator does.
  private updateSectionSelection(): void {
    this.panel
      .querySelectorAll('.slide-section-header')
      .forEach((header: Element) => {
        const sectionIndex = parseInt(
          (header as HTMLElement).dataset.sectionIndex as string,
          10,
        );
        const nameSpan = header.querySelector('.slide-section-name');
        if (!nameSpan) return;
        nameSpan.classList.toggle(
          'selected',
          this.isSectionFullySelected(sectionIndex),
        );
      });
  }

  // A blank picture with the slide's shape. Its intrinsic size gives the
  // frame the same footprint as the rendered thumbnail that replaces it,
  // so the list does not jump when the image arrives.
  private placeholderUrl(): string {
    const width = this.session.size.width > 0 ? this.session.size.width : 16;
    const height = this.session.size.height > 0 ? this.session.size.height : 9;
    return (
      'data:image/svg+xml,' +
      encodeURIComponent(
        '<svg xmlns="http://www.w3.org/2000/svg" width="' +
          width +
          '" height="' +
          height +
          '"/>',
      )
    );
  }

  // The slide carries the navigator's preview markup and classes, so
  // partsPreviewControl.css renders the thumbnails of both panels the
  // same way, in the strip and in the maximized grid alike. The corner
  // checkbox is a CSS-drawn picture of the option's selection state, so
  // it stays out of the accessibility tree; the option itself announces
  // that state through aria-selected.
  private renderSlide(index: number): HTMLElement {
    const session = this.session;
    const thumbnail = session.thumbnails.get(index);
    const selected = session.selection.has(index);
    return (
      <div
        class={
          'preview-frame slide-import-slide' +
          (this.isSlideCollapsed(index) ? ' section-collapsed' : '')
        }
        role="option"
        aria-selected={selected ? 'true' : 'false'}
        aria-label={this.slideLabel(index)}
        tabindex={index === this.focusIndex ? 0 : -1}
        data-index={index}
        draggable={this.selectedRemote ? 'false' : 'true'}
        onMouseDown={(e: MouseEvent) => this.onSlideMouseDown(e, index)}
        onClick={(e: MouseEvent) => this.onSlideClick(e, index)}
        onDragStart={(e: DragEvent) => this.onSlideDragStart(e, index)}
      >
        <span class="preview-helper" />
        <span class="preview-slide-number" aria-hidden="true" />
        <span class="slide-import-thumb">
          <img
            class={
              'preview-img' +
              (selected ? ' preview-img-selectedpart' : '') +
              (thumbnail ? '' : ' slide-import-skeleton')
            }
            src={thumbnail ? thumbnail.url : this.placeholderUrl()}
            alt=""
          />
          <span
            class={'slide-import-checkbox' + (selected ? ' checked' : '')}
            aria-hidden="true"
          />
        </span>
      </div>
    );
  }

  // Starts a drag that carries the slides to insert into the slides list.
  // Dragging a selected thumbnail carries the whole selection; dragging an
  // unselected one carries just that slide and leaves the selection alone.
  private onSlideDragStart(e: DragEvent, index: number): void {
    // A drag consumes the press, so no click follows it.
    this.pressedCheckboxIndex = -1;
    const session = this.session;
    const slides = session.selection.has(index)
      ? Array.from(session.selection).sort((a, b) => a - b)
      : [index];
    if (!e.dataTransfer) return;
    // The drag carries only the custom type. A drag that starts on the
    // picture is a native image drag, which the browser preloads with the
    // picture's URL as text and HTML; clearing that keeps a drop on a text
    // input or another application from pasting the thumbnail data there.
    e.dataTransfer.clearData();
    e.dataTransfer.setData(
      'application/x-cool-slide-import',
      JSON.stringify({ slides: slides }),
    );
    e.dataTransfer.effectAllowed = 'copy';
    this.setDragGhost(e, index, slides);
  }

  // The picture under the pointer during the drag: the grabbed thumbnail
  // on top, the other dragged slides stacked behind it, and a badge with
  // the count, like the slide navigator's reorder drag shows.
  private setDragGhost(e: DragEvent, index: number, slides: number[]): void {
    if (!e.dataTransfer.setDragImage) return;
    const frame = e.currentTarget as HTMLElement;
    const img = frame.querySelector('.preview-img') as HTMLImageElement | null;
    if (!img) return;
    const rect = img.getBoundingClientRect();
    const sources = [index]
      .concat(slides.filter((slide) => slide !== index))
      .map((slide) => {
        const thumbnail = this.session.thumbnails.get(slide);
        return thumbnail ? thumbnail.url : this.placeholderUrl();
      });
    const ghost = buildSlideDragGhost(
      sources,
      rect.width,
      rect.height,
      slides.length,
    );
    document.body.appendChild(ghost);
    e.dataTransfer.setDragImage(
      ghost,
      e.clientX - rect.left,
      e.clientY - rect.top,
    );
    // The snapshot is taken when the dragstart handler returns; the ghost
    // element itself is no longer needed after that.
    setTimeout(() => ghost.remove(), 0);
  }

  // Selects the slides between the anchor and the target that are on show;
  // the hidden slides of a collapsed section in between stay unselected, so
  // sweeping a range never picks up slides the user cannot see.
  private selectVisibleRange(anchor: number, target: number): void {
    const from = Math.min(anchor, target);
    const to = Math.max(anchor, target);
    this.session.setSelection(
      this.session.slides
        .map((slide) => slide.index)
        .filter(
          (index) =>
            index >= from && index <= to && !this.isSlideCollapsed(index),
        ),
    );
  }

  private onSlideMouseDown(e: MouseEvent, index: number): void {
    this.pressedCheckboxIndex =
      e.target instanceof Element &&
      e.target.closest('.slide-import-checkbox') !== null
        ? index
        : -1;
  }

  // Toggles one slide in and out of the selection and makes it the anchor
  // for a following range selection. The ctrl-click, space-key and corner
  // checkbox paths all toggle through here.
  private toggleSlide(index: number): void {
    this.anchorIndex = index;
    this.session.toggleSelection(index);
  }

  private onSlideClick(e: MouseEvent, index: number): void {
    // A click that starts or ends on the slide's corner checkbox toggles
    // the slide like a ctrl-click, so the mouse builds a multi-selection
    // without a modifier key. The click bubbles on like any other slide
    // click, and the keyboard focus follows it into the list.
    const onCheckbox =
      this.pressedCheckboxIndex === index ||
      (e.target instanceof Element &&
        e.target.closest('.slide-import-checkbox') !== null);
    this.pressedCheckboxIndex = -1;
    if (e.shiftKey) {
      this.selectVisibleRange(this.anchorIndex, index);
    } else if (e.ctrlKey || e.metaKey || onCheckbox) {
      this.toggleSlide(index);
    } else {
      this.anchorIndex = index;
      this.session.selectOnly(index);
    }
    this.setFocusIndex(index, true);
  }

  private onListKeyDown(e: KeyboardEvent): void {
    // The keyboard works on the slides on show; the slides of collapsed
    // sections are neither selected nor stepped onto.
    const visible = this.session.slides
      .map((slide) => slide.index)
      .filter((index) => !this.isSlideCollapsed(index));
    if (visible.length === 0) return;

    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'a') {
      this.session.setSelection(visible);
      e.preventDefault();
      return;
    }

    const position = visible.indexOf(this.focusIndex);

    let target = -1;
    if (e.key === 'ArrowDown')
      target = visible[Math.min(visible.length - 1, position + 1)];
    else if (e.key === 'ArrowUp') target = visible[Math.max(0, position - 1)];
    else if (e.key === 'Home') target = visible[0];
    else if (e.key === 'End') target = visible[visible.length - 1];

    if (target >= 0) {
      if (e.shiftKey) {
        this.selectVisibleRange(this.anchorIndex, target);
      } else if (!(e.ctrlKey || e.metaKey)) {
        this.anchorIndex = target;
        this.session.selectOnly(target);
      }
      this.setFocusIndex(target, true);
      e.preventDefault();
      return;
    }

    if (e.key === ' ' || e.key === 'Enter') {
      this.toggleSlide(this.focusIndex);
      e.preventDefault();
    }
  }

  private setFocusIndex(index: number, focus: boolean): void {
    this.focusIndex = index;
    this.panel
      .querySelectorAll('.slide-import-slide')
      .forEach((option: Element) => {
        const optionIndex = parseInt(
          (option as HTMLElement).dataset.index as string,
          10,
        );
        option.setAttribute('tabindex', optionIndex === index ? '0' : '-1');
        if (focus && optionIndex === index) {
          (option as HTMLElement).focus();
        }
      });
  }
}
