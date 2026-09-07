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
 * sidebar in a presentation.
 *
 * The pane lists the presentations this document is related to, shows the
 * slides of each of them under an expander, and inserts the slides the user
 * picked into the open document.
 *
 * The pane holds what every source answered about its slides. The import
 * itself - the selection, the options and the insert - lives in
 * SlideImportSession and belongs to the one source the selected slides come
 * from, the active source.
 */

/* global app _ JSDialog */

// What the pane holds of one source presentation.
interface SlideImportPaneSource {
  // The source's address, empty for a source the storage listed none for.
  wopiSrc: string;
  // Names the source in the pane. A source with no address is known by its
  // name alone.
  key: string;
  // Names the source's markup in a data attribute, so a selector finds it
  // whatever its address looks like.
  id: number;
  // The source as the user knows it.
  name: string;
  // The state the server reports for this view: available, subscribed,
  // connected, disconnected, failed, missing or noaccess.
  state: string;
  // Whether the source's slides are on show under its expander.
  expanded: boolean;
  // Whether the source has been asked for its slides since it connected.
  asked: boolean;
  // Whether the source has been asked to open, so that it is asked once
  // while the answer is on its way.
  opening: boolean;
  // The slide size of the source, in twips. Zero until it answers.
  docWidth: number;
  docHeight: number;
  // The part identifier of each slide, in slide order. A presentation names
  // its pages by these identifiers, not by their position, so a tile request
  // has to carry the identifier of the slide it wants.
  slideParts: string[];
  slides: SlideImportSlide[];
  sections: SlideImportSection[];
  thumbnails: Map<number, SlideImportThumbnail>;
  // Names of sections the user collapsed inside this source's panel.
  collapsedSections: Set<string>;
}

class SlideImportPane {
  private map: any;
  private session: SlideImportSession;
  private wrapper: HTMLElement;
  private panel: HTMLElement;
  private visible: boolean = false;

  // The sources on show, in the order the server lists them.
  private sources: SlideImportPaneSource[] = [];
  // Names the markup of the next source taken into the list.
  private nextSourceId: number = 0;
  // The source the selection and the insert belong to, by key, or empty when
  // no slide is picked yet.
  private activeKey: string = '';
  // Whether the pane has already opened a source of its own accord, which it
  // does for the first one of the list.
  private openedFirstSource: boolean = false;

  // Index of the slide frame that holds the roving tab stop, in the active
  // source's panel.
  private focusIndex: number = 0;
  // Anchor slide for shift-click and shift-arrow range selection.
  private anchorIndex: number = 0;
  // Index of the slide whose corner checkbox the primary button is pressed
  // on, or -1. The click handler reads it so a press that starts on the
  // checkbox toggles the slide even when the release lands beside it.
  private pressedCheckboxIndex: number = -1;

  // Whether the pages this pane asked a source to write are still to come.
  private awaitingExport: boolean = false;

  constructor(map: any) {
    this.map = map;
    this.session = new SlideImportSession(map);
    this.wrapper = document.getElementById(
      'slide-import-dock-wrapper',
    ) as HTMLElement;
    this.panel = document.getElementById('slide-import-panel') as HTMLElement;

    app.events.on('slideimport:statechange', this.redraw.bind(this));
    app.events.on('slideimport:error', this.redraw.bind(this));
    app.events.on('slideimport:selection', this.onSelectionChanged.bind(this));
    app.events.on('slideimport:inserted', this.onInserted.bind(this));
    app.events.on('slideimport:exportwanted', this.onExportWanted.bind(this));
    app.events.on('slidelink:changed', this.redraw.bind(this));
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
    this.refreshSources();
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
    this.sources = [];
    this.activeKey = '';
    this.openedFirstSource = false;
    this.panel.replaceChildren();
  }

  // While the panels are maximized the thumbnails flow into a wider grid.
  // The class is the one the slide navigator's wrapper carries in that
  // state, so one set of grid rules lays out both panels. Only an open pane
  // takes it: the class forces the wrapper on show, so it would reveal a
  // closed pane.
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

  // Something the pane shows changed: the state of the import, or the links
  // the source badges read.
  private redraw(): void {
    if (this.visible) this.render();
  }

  private onRelatedDocuments(): void {
    if (!this.visible) return;
    this.refreshSources();
    this.render();
  }

  // The source the selection belongs to, or null while none is picked.
  private activeSource(): SlideImportPaneSource | null {
    return this.sources.find((source) => source.key === this.activeKey) || null;
  }

  private isActive(source: SlideImportPaneSource): boolean {
    return source.key === this.activeKey && this.activeKey !== '';
  }

  private findSource(wopiSrc: string): SlideImportPaneSource | null {
    if (!wopiSrc) return null;
    return this.sources.find((source) => source.wopiSrc === wopiSrc) || null;
  }

  // Takes the related documents the server announced into the source list,
  // keeping what the sources already answered. A source that is no longer
  // connected has to be asked again, so what it answered before is dropped.
  private refreshSources(): void {
    const documents = app.relatedDocuments || [];
    const kept: SlideImportPaneSource[] = [];

    for (const doc of documents) {
      const name = SlideImportSession.documentName(doc);
      const key = doc.wopiSrc || 'name:' + name;
      let source = this.sources.find((known) => known.key === key);
      if (!source) {
        source = {
          wopiSrc: doc.wopiSrc || '',
          key: key,
          id: this.nextSourceId++,
          name: name,
          state: doc.state,
          expanded: false,
          asked: false,
          opening: false,
          docWidth: 0,
          docHeight: 0,
          slideParts: [],
          slides: [],
          sections: [],
          thumbnails: new Map(),
          collapsedSections: new Set(),
        };
      }
      source.name = name;
      source.state = doc.state;
      if (source.state !== 'connected' && source.asked) this.forget(source);
      kept.push(source);
    }

    this.sources = kept;

    // The selection belongs to a source that is gone or out of reach; there
    // is nothing left to insert.
    const active = this.activeSource();
    if (this.activeKey && (!active || active.state !== 'connected')) {
      this.activeKey = '';
      this.session.close();
    }

    // The first source of a document opens on its own, so the pane shows
    // slides as soon as it is opened.
    if (!this.openedFirstSource && this.sources.length) {
      this.openedFirstSource = true;
      this.expandSource(this.sources[0]);
    }

    for (const source of this.sources)
      if (source.expanded) this.askSource(source);
  }

  // Drops what a source answered about its slides.
  private forget(source: SlideImportPaneSource): void {
    source.asked = false;
    source.opening = false;
    source.docWidth = 0;
    source.docHeight = 0;
    source.slideParts = [];
    source.slides = [];
    source.sections = [];
    source.thumbnails.clear();
  }

  // Asks an open source for its slides: a source that is not connected yet is
  // subscribed to first, and the slides follow once it answers.
  private askSource(source: SlideImportPaneSource): void {
    if (!source.wopiSrc) return;

    if (source.state === 'connected') {
      if (source.asked) return;
      source.asked = true;
      this.sendRemoteCommand(source, 'getpresentationinfo');
      this.sendRemoteCommand(source, 'getslidesections');
      return;
    }

    if (
      source.state !== 'available' &&
      source.state !== 'disconnected' &&
      source.state !== 'failed'
    )
      return;

    if (source.opening) return;
    source.opening = true;
    SlideImportSession.subscribeRelatedDocument(source.wopiSrc);
  }

  private expandSource(source: SlideImportPaneSource): void {
    source.expanded = true;
    this.askSource(source);
  }

  private toggleSource(source: SlideImportPaneSource): void {
    if (source.expanded) source.expanded = false;
    else this.expandSource(source);
    this.render();
  }

  // Reads a source's slides again, so a file that changed shows what it holds
  // now.
  private reloadSource(source: SlideImportPaneSource): void {
    this.forget(source);
    if (this.isActive(source)) {
      this.session.slideCount = 0;
      this.session.clearSelection();
    }
    this.askSource(source);
    this.render();
  }

  // Makes a source the one the selection and the insert belong to. The
  // selection of the source left behind is dropped: an insert reads its
  // slides out of a single file.
  private activate(source: SlideImportPaneSource): void {
    if (this.isActive(source)) return;
    this.activeKey = source.key;
    this.focusIndex = 0;
    this.anchorIndex = 0;
    this.session.clearSelection();
    this.session.slideCount = source.slides.length;
    // The pages of a link insert record the document they came from, as the
    // user knows it.
    this.session.setSource(source.name);
    if (source.slides.length) this.session.slidesShown();
  }

  // The "Add presentation..." button asks the integration to open its own
  // file chooser, filtered to presentations, and to add the picked file as a
  // related document. The integration registers it with a POST to
  // /cool/relateddocument using the one-time token below. The added file then
  // arrives in the related documents list, from where its slides are
  // imported.
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

  // Sends a read-only client command to a subscribed source. Its reply
  // arrives as a remotedoccommandresult map event carrying the same wopiSrc.
  private sendRemoteCommand(
    source: SlideImportPaneSource,
    inner: string,
  ): void {
    SlideImportSession.sendRemoteCommand(source.wopiSrc, inner);
  }

  // A reply from one of the sources the pane shows.
  private onRemoteResult(e: {
    wopiSrc: string;
    textMsg: string;
    imgBytes?: Uint8Array;
    imgIndex?: number;
  }): void {
    const source = this.findSource(e.wopiSrc);
    if (!source) return;

    const textMsg = e.textMsg || '';
    if (textMsg.startsWith('presentationinfo:')) {
      this.onRemotePresentationInfo(
        source,
        textMsg.substring('presentationinfo:'.length),
      );
    } else if (textMsg.startsWith('slidesections:')) {
      this.onRemoteSlideSections(
        source,
        textMsg.substring('slidesections:'.length),
      );
    } else if (textMsg.startsWith('tile:')) {
      this.onRemoteThumbnail(source, textMsg, e.imgBytes, e.imgIndex ?? 0);
    } else if (textMsg.startsWith('exportslides:') && this.isActive(source)) {
      // Only the source an insert takes its slides from writes them out for
      // this pane.
      this.onRemoteExport(textMsg.substring('exportslides:'.length));
    }
  }

  // The slides a user chose are written out by the source as a presentation
  // of their own. The server stages it in this document's jail and names it
  // here, and the insert takes that file.
  private onExportWanted(): void {
    const source = this.activeSource();
    const insert = this.session.getPendingInsert();
    if (!source || !insert) return;

    const parts = insert.slides
      .map((index: number) => source.slideParts[index])
      .filter((part: string) => !!part);
    if (!parts.length) return;

    // Update Linked Slides asks the same document for its pages, so only the
    // export this pane asked for is taken as the file an insert reads.
    this.awaitingExport = true;
    this.sendRemoteCommand(source, 'exportslides slides=' + parts.join(','));
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

  private onRemotePresentationInfo(
    source: SlideImportPaneSource,
    json: string,
  ): void {
    try {
      const info = JSON.parse(json);
      source.docWidth = info.docWidth || 0;
      source.docHeight = info.docHeight || 0;
      const slides = Array.isArray(info.slides) ? info.slides : [];
      source.slideParts = slides.map((s: any) =>
        typeof s.part === 'string' ? s.part : '',
      );
      source.slides = slides.map((s: any, index: number) => ({
        index: index,
        name: '',
        guid: s.guid || '',
      }));
    } catch (err) {
      app.console.error('Bad remote presentationinfo: ' + err);
      return;
    }

    // The slides of the first source that answers are the ones an insert
    // takes, until the user picks slides of another source.
    if (!this.activeKey) {
      this.activate(source);
    } else if (this.isActive(source)) {
      this.session.slideCount = source.slides.length;
      // The slides are on show, so they can be chosen and inserted.
      this.session.slidesShown();
    }

    this.requestRemoteThumbnails(source);
    if (this.visible) this.render();
  }

  private onRemoteSlideSections(
    source: SlideImportPaneSource,
    json: string,
  ): void {
    try {
      const sections = JSON.parse(json);
      source.sections = Array.isArray(sections)
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

  // Asks a source for a picture of every slide, fitted to the pane width,
  // keyed by slot so each reply lands in its frame.
  private requestRemoteThumbnails(source: SlideImportPaneSource): void {
    if (
      source.slideParts.length <= 0 ||
      source.docWidth <= 0 ||
      source.docHeight <= 0
    )
      return;

    const targetWidth = 160;
    const targetHeight = Math.max(
      1,
      Math.round((targetWidth * source.docHeight) / source.docWidth),
    );
    const scale = app.roundedDpiScale || 1;

    for (let slot = 0; slot < source.slideParts.length; ++slot) {
      // The request names the slide by its part identifier, which the source
      // resolves to the page it paints. The slot index rides along as the
      // preview id: it is a non-negative integer, so the source treats the
      // request as a preview and echoes the id back, which lands the reply in
      // the right thumbnail slot.
      const partId = source.slideParts[slot];
      if (!partId) continue;
      this.sendRemoteCommand(
        source,
        'tile nviewid=0 part=' +
          partId +
          ' mode=0 width=' +
          Math.round(targetWidth * scale) +
          ' height=' +
          Math.round(targetHeight * scale) +
          ' tileposx=0 tileposy=0 tilewidth=' +
          source.docWidth +
          ' tileheight=' +
          source.docHeight +
          ' id=' +
          slot,
      );
    }
  }

  private onRemoteThumbnail(
    source: SlideImportPaneSource,
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
    if (isNaN(slot) || slot < 0 || slot >= source.slideParts.length) return;

    source.thumbnails.set(slot, {
      url: app.socket.pngDataUrl(imgBytes.subarray(imgIndex)),
      width: source.docWidth,
      height: source.docHeight,
    });

    // The slide frames already exist from the presentation info, so drop the
    // arrived thumbnail into its slot without rebuilding the list.
    this.updateThumbnail(source, slot);
  }

  // The source the pages of this document are linked to, when the given
  // source is one of them, and an empty string otherwise.
  private linkedSource(source: SlideImportPaneSource): string {
    const links = this.map.slideLinks;
    if (!links) return '';
    return links.linkedSourceOf({
      wopiSrc: source.wopiSrc,
      name: source.name,
    });
  }

  // What the pane says of a source: whether pages of this document are
  // linked to it, and when it cannot be reached, why.
  private sourceBadge(source: SlideImportPaneSource): {
    kind: string;
    label: string;
  } {
    if (source.state === 'missing')
      return { kind: 'missing', label: _('Missing') };
    if (source.state === 'noaccess')
      return { kind: 'missing', label: _('No access') };
    if (source.state === 'failed')
      return { kind: 'missing', label: _('Failed') };
    if (this.linkedSource(source))
      return { kind: 'linked', label: _('Linked') };
    return { kind: 'unlinked', label: _('Not linked') };
  }

  // Why a source shows no slides, or an empty string when it shows some.
  private sourceMessage(source: SlideImportPaneSource): string {
    if (source.slides.length) return '';
    if (source.state === 'missing') return _('This file is not available.');
    if (source.state === 'noaccess') return _('You cannot open this file.');
    if (source.state === 'failed') return _('This file could not be opened.');
    return _('Reading the slides...');
  }

  private onSelectionChanged(): void {
    if (!this.visible) return;
    const source = this.activeSource();
    const list = source ? this.slideList(source) : null;
    if (list && source) {
      list
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
      this.updateSectionSelection(source);
      this.updateSlideCount(source);
    }
    this.updateInsertButton();
  }

  private slideList(source: SlideImportPaneSource): HTMLElement | null {
    return this.panel.querySelector(
      '.slide-import-list[data-source-id="' + source.id + '"]',
    );
  }

  // Drops the thumbnail that arrived for a slide into its frame, replacing
  // the skeleton, without rebuilding the list.
  private updateThumbnail(source: SlideImportPaneSource, index: number): void {
    if (!this.visible) return;
    const list = this.slideList(source);
    const thumbnail = source.thumbnails.get(index);
    if (!list || !thumbnail) return;
    const option = list.querySelector(
      '.slide-import-slide[data-index="' + index + '"]',
    );
    if (!option) return;
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

  private slideLabel(source: SlideImportPaneSource, index: number): string {
    const slide = source.slides[index];
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

  private updateLinkByPosition(): void {
    const checkbox = this.panel.querySelector(
      '.slide-import-linkposition input',
    ) as HTMLInputElement | null;
    if (checkbox) checkbox.disabled = !this.session.linkToSource;
  }

  // How many slides a source holds, and how many of them are picked.
  private slideCountText(source: SlideImportPaneSource): string {
    const count = source.slides.length;
    if (!count) return '';
    const slides = _('{0} slides').replace('{0}', String(count));
    const picked = this.isActive(source) ? this.session.selection.size : 0;
    if (!picked) return slides;
    return slides + ' - ' + _('{0} selected').replace('{0}', String(picked));
  }

  private updateSlideCount(source: SlideImportPaneSource): void {
    const panel = this.panel.querySelector(
      '.slide-import-source-panel[data-source-id="' + source.id + '"]',
    );
    const count = panel
      ? panel.querySelector('.slide-import-panel-count')
      : null;
    if (count) count.textContent = this.slideCountText(source);
  }

  private statusText(): string {
    if (this.session.state === 'opening') return _('Opening file...');
    if (this.session.state === 'inserting') return _('Inserting slides...');
    return '';
  }

  private render(): void {
    const frame = this.panel.querySelector('.slide-import-panels');
    const scrollTop = frame ? frame.scrollTop : 0;

    const active = this.activeSource();
    const slideCount = active ? active.slides.length : 0;
    if (this.focusIndex >= slideCount) this.focusIndex = 0;
    if (this.anchorIndex >= slideCount) this.anchorIndex = 0;

    this.panel.replaceChildren(
      <div class="slide-import">
        {this.renderHeader()}
        {this.sources.length ? this.renderBody() : this.renderEmptyState()}
      </div>,
    );

    // The header buttons show their labels as tooltips, like the
    // navigator's header buttons do.
    this.panel
      .querySelectorAll('.navigation-expand-button, .close-navigation-button')
      .forEach((button: Element) =>
        window.L.control.attachTooltipEventListener(button, this.map),
      );

    const newFrame = this.panel.querySelector('.slide-import-panels');
    if (newFrame) newFrame.scrollTop = scrollTop;
  }

  // The header carries the slide navigator's header classes, so both panels
  // share one set of header, title and button styles.
  private renderHeader(): HTMLElement {
    // The rebuilt button carries the icon of the layout in hand, the one
    // the expander sets on every toggle.
    const expanded =
      this.map.paneExpander && this.map.paneExpander.getMode() === 'expanded';
    return (
      <div class="navigation-header">
        <h2 class="navigation-title slide-import-title">
          {_('Import slides')}
        </h2>
        <button
          class="navigation-expand-button slide-import-expand"
          aria-label={_('Expand slide import')}
          data-cooltip={_('Expand slide import')}
          aria-pressed={expanded ? 'true' : 'false'}
          onClick={() => this.toggleExpand()}
        >
          <img
            src={app.LOUtil.getImageURL(
              expanded ? 'collapse-slides.svg' : 'expand-slides.svg',
            )}
            alt=""
          />
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
    );
  }

  private renderAddButton(className: string): HTMLElement {
    return (
      <button
        class={className}
        disabled={!app.relatedDocumentToken}
        onClick={() => this.browseForImport()}
      >
        <span class="slide-import-add-plus" aria-hidden="true">
          +
        </span>
        {_('Add presentation...')}
      </button>
    );
  }

  // What a document with no presentation to import from shows: an invitation
  // to add one.
  private renderEmptyState(): HTMLElement {
    return (
      <div class="slide-import-empty">
        <div class="slide-import-empty-box">
          <span class="slide-import-empty-icon" aria-hidden="true">
            <img
              src={app.LOUtil.getImageURL('lc_presentinwindow.svg')}
              alt=""
            />
          </span>
          <div class="slide-import-empty-title">
            {_('No presentations yet')}
          </div>
          <div class="slide-import-empty-text">
            {_(
              'Add a presentation to browse its slides and insert them into this deck.',
            )}
          </div>
          {this.renderAddButton('button slide-import-add')}
        </div>
      </div>
    );
  }

  private renderBody(): HTMLElement {
    const session = this.session;
    const status = this.statusText();
    return (
      <div class="slide-import-body">
        <div class="slide-import-sources">
          <div class="slide-import-sources-title">
            {_('Recently imported from')}
          </div>
          <ul
            class="slide-import-source-list"
            aria-label={_('Presentations to import from')}
          >
            {this.sources.map((source) => this.renderSourceRow(source))}
          </ul>
          {this.renderAddButton('ui-linkbutton slide-import-add-link')}
        </div>
        <div class="slide-import-panels">
          {this.sources
            .filter((source) => this.canShowSlides(source))
            .map((source) => this.renderSourcePanel(source))}
        </div>
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
        {this.renderOptions()}
        <div class="slide-import-footer">
          <button
            class="button button-primary slide-import-insert"
            disabled={session.selection.size === 0 || session.state !== 'ready'}
            onClick={() => session.insertSelected()}
          >
            {this.insertButtonLabel()}
          </button>
        </div>
      </div>
    );
  }

  // Whether the pane can read the slides of a source. A file the storage
  // lists no address for cannot be reached at all.
  private canShowSlides(source: SlideImportPaneSource): boolean {
    return source.wopiSrc !== '';
  }

  // One row of the source list: the file, what the pane says of it, and the
  // menu of what can be done with it. The file is a button while there are
  // slides to show or hide, and the name alone when there are none.
  private renderSourceRow(source: SlideImportPaneSource): HTMLElement {
    const badge = this.sourceBadge(source);
    const file = [
      <img
        class="slide-import-source-icon"
        src={app.LOUtil.getImageURL('lc_presentinwindow.svg')}
        alt=""
      />,
      <span class="slide-import-source-name">{source.name}</span>,
    ];
    return (
      <li
        class={'slide-import-source' + (source.expanded ? ' expanded' : '')}
        data-state={source.state}
      >
        {this.canShowSlides(source) ? (
          <button
            class="slide-import-source-main"
            aria-expanded={source.expanded ? 'true' : 'false'}
            title={source.name}
            onClick={() => this.toggleSource(source)}
          >
            {file}
          </button>
        ) : (
          <span class="slide-import-source-main" title={source.name}>
            {file}
          </span>
        )}
        <span class={'slide-import-source-badge ' + badge.kind}>
          {badge.label}
        </span>
        <button
          class="slide-import-source-menu"
          aria-label={_('Actions for {0}').replace('{0}', source.name)}
          aria-haspopup="true"
          onClick={(e: MouseEvent) => this.openSourceMenu(e, source)}
        />
      </li>
    );
  }

  private openSourceMenu(e: MouseEvent, source: SlideImportPaneSource): void {
    const entries: any[] = [];

    // A file the storage lists no address for cannot be reached, so there is
    // nothing to do with it but name it to the integration, which is what
    // adding a presentation does.
    if (source.state === 'missing') {
      entries.push({
        id: 'locate',
        type: 'comboboxentry',
        text: _('Locate file'),
        enabled: !!app.relatedDocumentToken,
      });
      this.showSourceMenu(e, entries, source);
      return;
    }

    entries.push({
      id: 'toggle',
      type: 'comboboxentry',
      text: source.expanded ? _('Hide slides') : _('Show slides'),
    });

    // Reading the slides again also reaches a file that is not open yet, or
    // whose last read failed, so that is the way back from either.
    if (this.canShowSlides(source))
      entries.push({
        id: 'reload',
        type: 'comboboxentry',
        text: _('Reload slides'),
      });

    if (source.slides.length)
      entries.push({
        id: 'selectall',
        type: 'comboboxentry',
        text: _('Select all slides'),
      });

    const linked = this.linkedSource(source);
    if (linked)
      entries.push({
        id: 'update',
        type: 'comboboxentry',
        text: _('Update the slides linked to this file'),
      });

    this.showSourceMenu(e, entries, source);
  }

  private showSourceMenu(
    e: MouseEvent,
    entries: any[],
    source: SlideImportPaneSource,
  ): void {
    const linked = this.linkedSource(source);
    const callback = (
      objectType: any,
      eventType: string,
      object: any,
      data: any,
      entry: any,
    ) => {
      if (eventType !== 'selected') return false;
      switch (entry.id) {
        case 'locate':
          this.browseForImport();
          break;
        case 'toggle':
          this.toggleSource(source);
          break;
        case 'reload':
          this.reloadSource(source);
          break;
        case 'selectall':
          this.activate(source);
          this.session.setSelection(source.slides.map((slide) => slide.index));
          break;
        case 'update':
          this.map.slideLinks.updateSource(linked);
          break;
      }
      JSDialog.CloseAllDropdowns();
      return true;
    };

    JSDialog.OpenDropdown(
      'slide-import-source-menu',
      e.currentTarget as HTMLElement,
      entries,
      callback,
      '',
      false,
      false,
      true,
    );
  }

  // One source's slides under an expander, so several presentations are
  // browsed one after another without losing sight of the others.
  private renderSourcePanel(source: SlideImportPaneSource): HTMLElement {
    return (
      <div class="slide-import-source-panel" data-source-id={source.id}>
        <div
          class={
            'slide-section-header slide-import-panel-header' +
            (source.expanded ? '' : ' collapsed')
          }
          onClick={() => this.toggleSource(source)}
        >
          <button
            type="button"
            class="slide-section-toggle ui-expander-btn"
            aria-expanded={source.expanded ? 'true' : 'false'}
            aria-label={_('Toggle {0}').replace('{0}', source.name)}
            onClick={(event: MouseEvent) => {
              event.stopPropagation();
              this.toggleSource(source);
            }}
          />
          <span class="slide-section-name" title={source.name}>
            {source.name}
          </span>
          <span class="slide-import-panel-count">
            {this.slideCountText(source)}
          </span>
        </div>
        {source.expanded && this.renderPanelBody(source)}
      </div>
    );
  }

  private renderPanelBody(source: SlideImportPaneSource): HTMLElement {
    const message = this.sourceMessage(source);
    if (message) return <div class="slide-import-panel-message">{message}</div>;

    return (
      <div
        class="slide-import-list"
        role="listbox"
        aria-multiselectable="true"
        data-source-id={source.id}
        aria-label={_('Slides in {0}').replace('{0}', source.name)}
        onKeyDown={(e: KeyboardEvent) => this.onListKeyDown(e, source)}
      >
        {this.renderListItems(source)}
      </div>
    );
  }

  private renderOptions(): HTMLElement {
    const session = this.session;
    return (
      <div class="slide-import-options">
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
        <label class="slide-import-linksource">
          <input
            type="checkbox"
            checked={session.linkToSource}
            disabled={!session.canLink}
            onChange={(e: Event) => {
              session.setLinkToSource((e.target as HTMLInputElement).checked);
              this.updateLinkByPosition();
            }}
          />
          {_('Link to the source file')}
        </label>
        <label class="slide-import-linkposition">
          <input
            type="checkbox"
            checked={session.linkByPosition}
            disabled={!session.canLink || !session.linkToSource}
            onChange={(e: Event) =>
              session.setLinkByPosition((e.target as HTMLInputElement).checked)
            }
          />
          {_('Link slides to the position in a source file')}
        </label>
      </div>
    );
  }

  // The slides of one source in order, with a header before the first slide
  // of each section, like the slide navigator shows the document's own
  // sections. Slides and sections are both in slide order, so one pointer
  // walks the sections along with the slides; consecutive sections can share
  // a start slide when a section is empty, and each still gets its own
  // header.
  private renderListItems(source: SlideImportPaneSource): HTMLElement[] {
    const items: HTMLElement[] = [];
    let nextSection = 0;
    source.slides.forEach((slide) => {
      while (
        nextSection < source.sections.length &&
        source.sections[nextSection].startIndex <= slide.index
      ) {
        items.push(this.renderSectionHeader(source, nextSection));
        nextSection++;
      }
      items.push(this.renderSlide(source, slide.index));
    });
    return items;
  }

  // The section a slide belongs to: the last section starting at or before
  // it. Slides in front of the first section belong to no section.
  private sectionOf(
    source: SlideImportPaneSource,
    index: number,
  ): SlideImportSection | null {
    let result: SlideImportSection | null = null;
    for (const section of source.sections) {
      if (section.startIndex > index) break;
      result = section;
    }
    return result;
  }

  private isSlideCollapsed(
    source: SlideImportPaneSource,
    index: number,
  ): boolean {
    const section = this.sectionOf(source, index);
    return section !== null && source.collapsedSections.has(section.name);
  }

  private isSectionFullySelected(
    source: SlideImportPaneSource,
    sectionIndex: number,
  ): boolean {
    if (!this.isActive(source)) return false;
    const section = source.sections[sectionIndex];
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
  private renderSectionHeader(
    source: SlideImportPaneSource,
    sectionIndex: number,
  ): HTMLElement {
    const section = source.sections[sectionIndex];
    const header = buildSlideSectionHeader(
      section.name,
      sectionIndex,
      source.collapsedSections.has(section.name),
      {
        onToggle: () => this.toggleSectionCollapse(source, sectionIndex),
        onSelect: () => this.selectSection(source, sectionIndex),
      },
      _('{0} slides').replace('{0}', String(section.slideCount)),
    );
    if (this.isSectionFullySelected(source, sectionIndex))
      header.querySelector('.slide-section-name').classList.add('selected');
    return header;
  }

  private toggleSectionCollapse(
    source: SlideImportPaneSource,
    sectionIndex: number,
  ): void {
    const section = source.sections[sectionIndex];
    if (!section) return;
    if (source.collapsedSections.has(section.name))
      source.collapsedSections.delete(section.name);
    else source.collapsedSections.add(section.name);
    // A hidden slide cannot hold the list's tab stop; hand it to the
    // first slide still on show.
    if (
      this.isActive(source) &&
      this.isSlideCollapsed(source, this.focusIndex)
    ) {
      const firstVisible = source.slides
        .map((slide) => slide.index)
        .find((index) => !this.isSlideCollapsed(source, index));
      if (firstVisible !== undefined) this.focusIndex = firstVisible;
    }
    this.render();
    // The rebuild replaced the pressed chevron; keyboard focus continues on
    // its replacement.
    const list = this.slideList(source);
    const toggle = list
      ? (list.querySelector(
          '.slide-section-header[data-section-index="' +
            sectionIndex +
            '"] .slide-section-toggle',
        ) as HTMLElement | null)
      : null;
    if (toggle) toggle.focus();
  }

  // Clicking a header selects every slide of its section, like the slide
  // navigator does.
  private selectSection(
    source: SlideImportPaneSource,
    sectionIndex: number,
  ): void {
    const section = source.sections[sectionIndex];
    if (!section || section.slideCount <= 0) return;
    this.activate(source);
    const start = section.startIndex;
    this.anchorIndex = start;
    this.session.selectRange(start, start + section.slideCount - 1);
    // A collapsed section's slides are hidden and cannot hold the list's
    // tab stop, so it stays where it is.
    if (!this.isSlideCollapsed(source, start)) this.setFocusIndex(start, false);
  }

  // A section whose slides are all selected shows its name highlighted,
  // like the slide navigator does.
  private updateSectionSelection(source: SlideImportPaneSource): void {
    const list = this.slideList(source);
    if (!list) return;
    list
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
          this.isSectionFullySelected(source, sectionIndex),
        );
      });
  }

  // A blank picture with the slide's shape. Its intrinsic size gives the
  // frame the same footprint as the rendered thumbnail that replaces it,
  // so the list does not jump when the image arrives.
  private placeholderUrl(source: SlideImportPaneSource): string {
    const width = source.docWidth > 0 ? source.docWidth : 16;
    const height = source.docHeight > 0 ? source.docHeight : 9;
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
  // same way, in the docked pane and in the maximized grid alike. The corner
  // checkbox is a CSS-drawn picture of the option's selection state, so
  // it stays out of the accessibility tree; the option itself announces
  // that state through aria-selected.
  private renderSlide(
    source: SlideImportPaneSource,
    index: number,
  ): HTMLElement {
    const thumbnail = source.thumbnails.get(index);
    const active = this.isActive(source);
    const selected = active && this.session.selection.has(index);
    const focused = active ? index === this.focusIndex : index === 0;
    return (
      <div
        class={
          'preview-frame slide-import-slide' +
          (this.isSlideCollapsed(source, index) ? ' section-collapsed' : '')
        }
        role="option"
        aria-selected={selected ? 'true' : 'false'}
        aria-label={this.slideLabel(source, index)}
        tabindex={focused ? 0 : -1}
        data-index={index}
        draggable={active ? 'true' : 'false'}
        onMouseDown={(e: MouseEvent) => this.onSlideMouseDown(e, index)}
        onClick={(e: MouseEvent) => this.onSlideClick(e, source, index)}
        onDragStart={(e: DragEvent) => this.onSlideDragStart(e, source, index)}
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
            src={thumbnail ? thumbnail.url : this.placeholderUrl(source)}
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
  private onSlideDragStart(
    e: DragEvent,
    source: SlideImportPaneSource,
    index: number,
  ): void {
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
    this.setDragGhost(e, source, index, slides);
  }

  // The picture under the pointer during the drag: the grabbed thumbnail
  // on top, the other dragged slides stacked behind it, and a badge with
  // the count, like the slide navigator's reorder drag shows.
  private setDragGhost(
    e: DragEvent,
    source: SlideImportPaneSource,
    index: number,
    slides: number[],
  ): void {
    if (!e.dataTransfer.setDragImage) return;
    const frame = e.currentTarget as HTMLElement;
    const img = frame.querySelector('.preview-img') as HTMLImageElement | null;
    if (!img) return;
    const rect = img.getBoundingClientRect();
    const sources = [index]
      .concat(slides.filter((slide) => slide !== index))
      .map((slide) => {
        const thumbnail = source.thumbnails.get(slide);
        return thumbnail ? thumbnail.url : this.placeholderUrl(source);
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
  private selectVisibleRange(
    source: SlideImportPaneSource,
    anchor: number,
    target: number,
  ): void {
    const from = Math.min(anchor, target);
    const to = Math.max(anchor, target);
    this.session.setSelection(
      source.slides
        .map((slide) => slide.index)
        .filter(
          (index) =>
            index >= from &&
            index <= to &&
            !this.isSlideCollapsed(source, index),
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

  private onSlideClick(
    e: MouseEvent,
    source: SlideImportPaneSource,
    index: number,
  ): void {
    // A click that starts or ends on the slide's corner checkbox toggles
    // the slide like a ctrl-click, so the mouse builds a multi-selection
    // without a modifier key. The click bubbles on like any other slide
    // click, and the keyboard focus follows it into the list.
    const onCheckbox =
      this.pressedCheckboxIndex === index ||
      (e.target instanceof Element &&
        e.target.closest('.slide-import-checkbox') !== null);
    this.pressedCheckboxIndex = -1;

    // Picking a slide of another source starts a selection of that source.
    const switched = !this.isActive(source);
    this.activate(source);

    if (e.shiftKey && !switched) {
      this.selectVisibleRange(source, this.anchorIndex, index);
    } else if ((e.ctrlKey || e.metaKey || onCheckbox) && !switched) {
      this.toggleSlide(index);
    } else {
      this.anchorIndex = index;
      this.session.selectOnly(index);
    }
    this.setFocusIndex(index, true);
  }

  private onListKeyDown(e: KeyboardEvent, source: SlideImportPaneSource): void {
    // The keyboard works on the slides on show; the slides of collapsed
    // sections are neither selected nor stepped onto.
    const visible = source.slides
      .map((slide) => slide.index)
      .filter((index) => !this.isSlideCollapsed(source, index));
    if (visible.length === 0) return;

    // The keyboard picks slides of the source it is in, so a key press
    // starts a selection of that source.
    this.activate(source);

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
        this.selectVisibleRange(source, this.anchorIndex, target);
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
    const source = this.activeSource();
    const list = source ? this.slideList(source) : null;
    if (!list) return;
    list.querySelectorAll('.slide-import-slide').forEach((option: Element) => {
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
