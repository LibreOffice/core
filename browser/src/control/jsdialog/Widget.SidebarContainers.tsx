// @ts-strict-ignore
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

/*
 * JSDialog.SidebarContainers - various container widgets for desktop sidebar
 */

declare var JSDialog: any;

JSDialog.deck = function (
  parentContainer: Element,
  data: DeckWidgetJSON,
  builder: JSBuilder,
) {
  var deck = window.L.DomUtil.create(
    'div',
    'deck ' + builder.options.cssClass,
    parentContainer,
  );
  deck.id = data.id;
  deck.tabIndex = '-1';

  if (data.headerText) {
    deck.appendChild(
      <div class={'ui-panel-header ui-deck-header ' + builder.options.cssClass}>
        <h2
          class={'ui-panel-title ' + builder.options.cssClass}
          id={data.id + '-header-label'}
        >
          {data.headerText}
        </h2>
      </div>,
    );
  }

  for (var i = 0; i < data.children.length; i++) {
    builder.build(deck, [data.children[i]], undefined);
  }

  return false;
};

interface OverlaySidebarPanelOptions {
  id: string;
  title: string;
  cssClass: string;
  map: any;
  onClose: () => void;
}

interface OverlaySidebarPanelParts {
  container: HTMLElement;
  header: HTMLElement;
  closeWrapper: HTMLElement;
  content: HTMLElement;
}

// builds the frame an overlay sidebar deck is made of
JSDialog.buildOverlaySidebarPanel = function (
  options: OverlaySidebarPanelOptions,
): OverlaySidebarPanelParts {
  const closeText = _('Close');
  const labelId = options.id + '-label';
  let content: HTMLElement;
  let header: HTMLElement;
  let closeWrapper: HTMLElement;

  const container = (
    <div class={'ui-panel-container ' + options.cssClass} id={options.id}>
      <div
        class={'ui-panel-header ' + options.cssClass}
        ref={(el: HTMLElement) => {
          header = el;
        }}
      >
        <h2 class={'ui-panel-title ' + options.cssClass} id={labelId}>
          {options.title}
        </h2>
        <div
          class="close-navigation-wrapper"
          ref={(el: HTMLElement) => {
            closeWrapper = el;
          }}
        >
          <button
            class="close-navigation-button ui-panel-close-button"
            id={options.id + '-close-button'}
            aria-label={closeText}
            data-cooltip={closeText}
            onClick={() => options.onClose()}
            ref={(el: HTMLElement) =>
              window.L.control.attachTooltipEventListener(el, options.map)
            }
          ></button>
        </div>
      </div>
      <div
        class={'ui-panel-content ' + options.cssClass}
        id={options.id + '-children'}
        role="region"
        aria-labelledby={labelId}
        ref={(el: HTMLElement) => {
          content = el;
        }}
      ></div>
    </div>
  );

  return { container, header, closeWrapper, content };
};

// builds the single section of an overlay deck
function buildPanelWithHeaderAction(
  parentContainer: Element,
  data: PanelWidgetJSON,
  builder: JSBuilder,
) {
  const { container, header, closeWrapper, content } =
    JSDialog.buildOverlaySidebarPanel({
      id: data.id,
      title: builder._cleanText(data.text),
      cssClass: builder.options.cssClass,
      map: builder.map,
      onClose: () => builder.map.sendUnoCommand(data.closeCommand),
    });

  // The panel keeps its options menu next to the close button, at the leading
  // side of the close button so the close button stays at the trailing edge.
  if (data.command) {
    const icon = window.L.DomUtil.create(
      'div',
      'ui-expander-icon-right ' + builder.options.cssClass,
    );
    header.insertBefore(icon, closeWrapper);
    const moreOptionsText = data.text
      ? _('More options for {1}').replace('{1}', data.text)
      : '';
    builder._controlHandlers['toolitem'](
      icon,
      {
        type: 'toolitem',
        command: data.command,
        aria: { label: moreOptionsText, role: 'popup' },
        icon: app.LOUtil.getIconNameOfCommand('morebutton'),
        tooltip: moreOptionsText,
      } as any as WidgetJSON, // FIXME: use toolitem JSON type
      builder,
    );
  }

  if (data.name) container.classList.add(data.name);
  if (data.hidden === true) container.classList.add('hidden');
  parentContainer.appendChild(container);

  builder.build(content, data.children, undefined);
}

JSDialog.panel = function (
  parentContainer: Element,
  data: PanelWidgetJSON,
  builder: JSBuilder,
) {
  if (data.closeCommand) {
    buildPanelWithHeaderAction(parentContainer, data, builder);
    return false;
  }

  var expanderData: ExpanderWidgetJSON = data;
  expanderData.type = 'expander';
  expanderData.children = ([{ text: data.text }] as Array<any>).concat(
    data.children,
  );

  builder._controlHandlers['expander'](
    parentContainer,
    expanderData,
    builder,
    () => {
      /*do nothing*/
    },
  );

  var expander = $(parentContainer).children('#' + expanderData.id);

  if (data.name) window.L.DomUtil.addClass(expander.get(0), data.name);
  if (expanderData.hidden === true) expander.hide();

  if (expanderData.command) {
    var iconParent = expander.children('.ui-expander').get(0);
    var icon = window.L.DomUtil.create(
      'div',
      'ui-expander-icon-right ' + builder.options.cssClass,
      iconParent,
    );
    const moreOptionsText = expanderData.children[0].text
      ? _('More options for {1}').replace('{1}', expanderData.children[0].text)
      : '';
    builder._controlHandlers['toolitem'](
      icon,
      {
        type: 'toolitem',
        command: expanderData.command,
        aria: { label: moreOptionsText, role: 'popup' },
        icon: app.LOUtil.getIconNameOfCommand('morebutton'),
        tooltip: moreOptionsText,
      } as any as WidgetJSON, // FIXME: use toolitem JSON type
      builder,
    );
  }

  return false;
};
