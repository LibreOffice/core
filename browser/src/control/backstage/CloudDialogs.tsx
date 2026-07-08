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
/* global _ */

/*
 * Modal dialogs for the cloud-provider flow: add/edit, the post-add
 * success dialog, and the remove confirmation. Each function returns the
 * overlay element; the caller appends it and calls .remove() to dismiss.
 * Also exports the custom kind dropdown widget.
 */

namespace BackstageTemplates {
  /** Generic modal frame. Use as a JSX component. */
  export interface ModalProps {
    titleId: string;
    title: string;
    onDismiss: () => void;
    children?: any;
  }

  export function modalShell(props: ModalProps): HTMLElement {
    return (
      <div
        class="backstage-modal-overlay"
        onClick={(e: Event) => {
          if (e.target === e.currentTarget) props.onDismiss();
        }}
      >
        <div
          class="backstage-modal"
          role="dialog"
          aria-modal="true"
          aria-labelledby={props.titleId}
        >
          <h2 class="backstage-modal-title" id={props.titleId}>
            {props.title}
          </h2>
          {props.children}
        </div>
      </div>
    );
  }

  // ---- Add / Edit dialog ----

  export interface AddCloudDialogProps {
    isEdit: boolean;
    initialKind: CloudProviderKind;
    initialDomain: string;
    initialName: string;
    onCancel: () => void;
    onSubmit: (kind: CloudProviderKind, name: string, domain: string) => void;
    onDelete?: () => void;
  }

  export function addCloudDialog(props: AddCloudDialogProps): HTMLElement {
    let domainInput: HTMLInputElement;
    let nameInput: HTMLInputElement;
    const dropdown = kindDropdown(
      props.initialKind,
      'backstage-add-cloud-type-label',
    );

    const submit = (e: Event) => {
      e.preventDefault();
      const domain = domainInput.value.trim();
      const name = nameInput.value.trim();
      if (!domain) {
        domainInput.focus();
        return;
      }
      if (!name) {
        nameInput.focus();
        return;
      }
      props.onSubmit(dropdown.getValue(), name, domain);
    };

    const overlay = modalShell({
      titleId: 'backstage-add-cloud-title',
      title: props.isEdit ? _('Edit account') : _('Add new account'),
      onDismiss: props.onCancel,
      children: (
        <form class="backstage-modal-form" novalidate onSubmit={submit}>
          <div class="backstage-modal-field">
            <span
              class="backstage-modal-field-label"
              id="backstage-add-cloud-type-label"
            >
              {_('Type')}
            </span>
            {dropdown.element}
          </div>
          <label class="backstage-modal-field">
            <span class="backstage-modal-field-label">{_('Server URL')}</span>
            <input
              class="backstage-modal-input ui-edit"
              type="text"
              required
              placeholder={_('Enter server URL')}
              value={props.initialDomain}
              ref={(el: HTMLInputElement) => {
                domainInput = el;
              }}
            />
          </label>
          <label class="backstage-modal-field">
            <span class="backstage-modal-field-label">{_('Display name')}</span>
            <input
              class="backstage-modal-input ui-edit"
              type="text"
              required
              maxlength="40"
              placeholder={_('Enter display name')}
              value={props.initialName}
              ref={(el: HTMLInputElement) => {
                nameInput = el;
              }}
            />
            <span class="backstage-modal-field-hint">
              {_('Name displayed in your cloud list (max. 40 characters).')}
            </span>
          </label>

          {props.isEdit && props.onDelete
            ? dangerSection(props.onDelete)
            : null}

          <div class="backstage-modal-actions">
            <button type="button" class="button" onClick={props.onCancel}>
              {_('Cancel')}
            </button>
            <button type="submit" class="button button-primary">
              {props.isEdit ? _('Save changes') : _('Add Account')}
            </button>
          </div>
        </form>
      ),
    });

    setTimeout(() => domainInput?.focus(), 0);
    return overlay;
  }

  function providerPreviewRow(props: {
    typeName: string;
    iconName: string;
    userName: string;
  }): HTMLElement {
    const svg = BackstageSVGIcons[props.iconName];
    return (
      <div class="backstage-modal-provider-row" aria-hidden="true">
        <span
          class="backstage-modal-provider-row-icon"
          dangerouslySetInnerHTML={svg ? { __html: svg } : undefined}
        />
        <div class="backstage-modal-provider-row-text">
          <span class="backstage-modal-provider-row-type">
            {props.typeName}
          </span>
          <span class="backstage-modal-provider-row-name">
            {props.userName}
          </span>
        </div>
      </div>
    );
  }

  function dangerSection(onDelete: () => void): HTMLElement {
    return (
      <div class="backstage-modal-danger-section">
        <div class="backstage-modal-danger-text">
          <h3 class="backstage-modal-danger-heading">{_('Remove account')}</h3>
          <span class="backstage-modal-danger-subtitle">
            {_('Disconnects this cloud')}
          </span>
        </div>
        <button
          type="button"
          class="button button-danger-outline"
          onClick={onDelete}
        >
          {_('Delete')}
        </button>
      </div>
    );
  }

  // ---- Cloud-added success dialog ----

  export interface CloudAddedDialogProps {
    typeName: string;
    iconName: string;
    userName: string;
    onClose: () => void;
    onOpen: () => void;
  }

  export function cloudAddedDialog(props: CloudAddedDialogProps): HTMLElement {
    let openBtn: HTMLButtonElement;
    const overlay = modalShell({
      titleId: 'backstage-cloud-added-title',
      title: _('New account successfully added'),
      onDismiss: props.onClose,
      children: (
        <>
          <p class="backstage-modal-subtitle">
            {_('You can now access your files from this cloud.')}
          </p>
          <div class="backstage-modal-preview">
            {openTile({
              label: props.typeName,
              iconName: props.iconName,
              subtitle: props.userName,
              extraClass: 'is-cloud-provider is-preview',
              inert: true,
            })}
          </div>
          <div class="backstage-modal-actions">
            <button type="button" class="button" onClick={props.onClose}>
              {_('Close')}
            </button>
            <button
              type="button"
              class="button button-primary"
              onClick={props.onOpen}
              ref={(el: HTMLButtonElement) => {
                openBtn = el;
              }}
            >
              {_('Open Files')}
            </button>
          </div>
        </>
      ),
    });
    setTimeout(() => openBtn?.focus(), 0);
    return overlay;
  }

  // ---- Remove confirmation dialog ----

  export interface RemoveConfirmDialogProps {
    typeName: string;
    iconName: string;
    userName: string;
    onCancel: () => void;
    onConfirm: () => void;
  }

  export function removeConfirmDialog(
    props: RemoveConfirmDialogProps,
  ): HTMLElement {
    let cancelBtn: HTMLButtonElement;
    const overlay = modalShell({
      titleId: 'backstage-remove-confirm-title',
      title: _('Remove this account?'),
      onDismiss: props.onCancel,
      children: (
        <>
          <p class="backstage-modal-subtitle">
            {_("You'll be disconnected from this cloud account.")}
          </p>
          {providerPreviewRow({
            typeName: props.typeName,
            iconName: props.iconName,
            userName: props.userName,
          })}
          <div class="backstage-modal-actions">
            <button
              type="button"
              class="button"
              onClick={props.onCancel}
              ref={(el: HTMLButtonElement) => {
                cancelBtn = el;
              }}
            >
              {_('Cancel')}
            </button>
            <button
              type="button"
              class="button button-danger"
              onClick={props.onConfirm}
            >
              {_('Remove')}
            </button>
          </div>
        </>
      ),
    });
    setTimeout(() => cancelBtn?.focus(), 0);
    return overlay;
  }

  // ---- Custom kind dropdown widget ----
  //
  // Returns the element plus a getValue() reader. State (selected value,
  // open/closed, click-outside listener) is encapsulated in this closure.

  export function kindDropdown(
    initialValue: CloudProviderKind,
    labelledBy?: string,
  ): { element: HTMLElement; getValue: () => CloudProviderKind } {
    const kinds: { value: CloudProviderKind; label: string }[] = [
      { value: 'nextcloud', label: _('Nextcloud') },
      { value: 'opencloud', label: _('OpenCloud') },
      { value: 'seafile', label: _('Seafile') },
      { value: 'other', label: _('Other') },
    ];

    let currentValue: CloudProviderKind = initialValue;
    let isOpen = false;
    let wrapperEl: HTMLElement;
    let toggleEl: HTMLButtonElement;
    let toggleLabelEl: HTMLElement;
    let toggleIconEl: HTMLElement;
    const optionEls: HTMLElement[] = [];

    const setIconContent = (el: HTMLElement, kind: CloudProviderKind) => {
      const svg = BackstageSVGIcons[iconForKind(kind)];
      el.innerHTML = svg || '';
    };

    const updateToggle = () => {
      const k = kinds.find((entry) => entry.value === currentValue);
      toggleLabelEl.textContent = k ? k.label : '';
      setIconContent(toggleIconEl, currentValue);
    };

    const onDocMouseDown = (e: MouseEvent) => {
      if (isOpen && !wrapperEl.contains(e.target as Node)) close();
    };

    const open = () => {
      if (isOpen) return;
      isOpen = true;
      wrapperEl.classList.add('is-open');
      toggleEl.setAttribute('aria-expanded', 'true');
      document.addEventListener('mousedown', onDocMouseDown, true);
      const selected = optionEls.find(
        (el) => el.getAttribute('data-value') === currentValue,
      );
      selected?.focus();
    };

    const close = () => {
      if (!isOpen) return;
      isOpen = false;
      wrapperEl.classList.remove('is-open');
      toggleEl.setAttribute('aria-expanded', 'false');
      document.removeEventListener('mousedown', onDocMouseDown, true);
    };

    const select = (k: CloudProviderKind) => {
      currentValue = k;
      updateToggle();
      optionEls.forEach((el) =>
        el.setAttribute(
          'aria-selected',
          String(el.getAttribute('data-value') === currentValue),
        ),
      );
      close();
      toggleEl.focus();
    };

    const element = (
      <div
        class="backstage-modal-dropdown"
        ref={(el: HTMLElement) => {
          wrapperEl = el;
        }}
        onKeydown={(e: KeyboardEvent) => {
          if (e.key === 'Escape' && isOpen) {
            e.preventDefault();
            close();
            toggleEl.focus();
          }
        }}
      >
        <button
          type="button"
          class="backstage-modal-dropdown-toggle"
          aria-haspopup="listbox"
          aria-expanded="false"
          aria-labelledby={labelledBy}
          ref={(el: HTMLButtonElement) => {
            toggleEl = el;
          }}
          onClick={() => (isOpen ? close() : open())}
        >
          <span
            class="backstage-modal-dropdown-icon"
            aria-hidden="true"
            ref={(el: HTMLElement) => {
              toggleIconEl = el;
            }}
          />
          <span
            class="backstage-modal-dropdown-label"
            ref={(el: HTMLElement) => {
              toggleLabelEl = el;
            }}
          />
          <span class="backstage-modal-dropdown-arrow" aria-hidden="true">
            ▾
          </span>
        </button>
        <ul class="backstage-modal-dropdown-list" role="listbox">
          {kinds.map((k) => {
            const optIconHtml = BackstageSVGIcons[iconForKind(k.value)] || '';
            return (
              <li
                class="backstage-modal-dropdown-option"
                role="option"
                data-value={k.value}
                tabindex="0"
                aria-selected={String(k.value === currentValue)}
                ref={(el: HTMLElement) => optionEls.push(el)}
                onClick={() => select(k.value)}
                onKeydown={(e: KeyboardEvent) => {
                  if (e.key === 'Enter' || e.key === ' ') {
                    e.preventDefault();
                    select(k.value);
                  }
                }}
              >
                <span
                  class="backstage-modal-dropdown-icon"
                  aria-hidden="true"
                  dangerouslySetInnerHTML={{ __html: optIconHtml }}
                />
                <span>{k.label}</span>
              </li>
            );
          })}
        </ul>
      </div>
    );

    updateToggle();

    return {
      element,
      getValue: () => currentValue,
    };
  }

  function iconForKind(kind: CloudProviderKind): string {
    switch (kind) {
      case 'nextcloud':
        return 'nextcloud.svg';
      case 'opencloud':
        return 'opencloud.svg';
      case 'seafile':
        return 'seafile.svg';
      case 'other':
        return 'generic.svg';
    }
  }
}
