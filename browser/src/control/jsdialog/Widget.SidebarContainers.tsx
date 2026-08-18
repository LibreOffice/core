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
  data: WidgetJSON,
  builder: JSBuilder,
) {
  var deck = window.L.DomUtil.create(
    'div',
    'deck ' + builder.options.cssClass,
    parentContainer,
  );
  deck.id = data.id;
  deck.tabIndex = '-1';

  for (var i = 0; i < data.children.length; i++) {
    builder.build(deck, [data.children[i]], undefined);
  }

  return false;
};

// Builds a panel header without the expand toggle; the panel content is always visible. The
// header carries the way out of the deck instead: a close button at the trailing edge that
// sends the panel's closeCommand.
function buildPanelWithHeaderAction(
  parentContainer: Element,
  data: PanelWidgetJSON,
  builder: JSBuilder,
) {
  const closeText = _('Close');
  const labelId = data.id + '-label';
  let content: HTMLElement;
  let header: HTMLElement;
  let closeWrapper: HTMLElement;

  const container = (
    <div
      class={'ui-expander-container ' + builder.options.cssClass}
      id={data.id}
    >
      <div
        class={'ui-expander ' + builder.options.cssClass}
        ref={(el: HTMLElement) => {
          header = el;
        }}
      >
        <h2 class={'ui-expander-heading ' + builder.options.cssClass}>
          <span
            class={
              'ui-expander-label ui-panel-plain-label ' +
              builder.options.cssClass
            }
            id={labelId}
          >
            {builder._cleanText(data.text)}
          </span>
        </h2>
        <div
          class="close-navigation-wrapper ui-panel-close-wrapper"
          ref={(el: HTMLElement) => {
            closeWrapper = el;
          }}
        >
          <button
            class="close-navigation-button ui-panel-close-button"
            id={data.id + '-close-button'}
            aria-label={closeText}
            data-cooltip={closeText}
            onClick={() => builder.map.sendUnoCommand(data.closeCommand)}
            ref={(el: HTMLElement) =>
              window.L.control.attachTooltipEventListener(el, builder.map)
            }
          ></button>
        </div>
      </div>
      <div
        class={'ui-expander-content expanded ' + builder.options.cssClass}
        id={data.id + '-children'}
        role="region"
        aria-labelledby={labelId}
        ref={(el: HTMLElement) => {
          content = el;
        }}
      ></div>
    </div>
  );

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
