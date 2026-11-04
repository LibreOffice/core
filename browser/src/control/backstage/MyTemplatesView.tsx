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
/* global _ formatByteSize */

/*
 * The templates that the user added: the My templates section of the New view, the Manage
 * templates view, and the dialogs that add, rename and remove a template.
 */

namespace BackstageTemplates {
  export interface MyTemplatesSectionProps {
    templates: TemplateData[];
    onTemplateClick: (template: TemplateData) => void;
    onAddClick: () => void;
    onManageClick: () => void;
    favoriteState: (template: TemplateData) => TemplateFavoriteState;
  }

  export interface ManageTemplatesProps {
    // False when the user has added no template yet.
    hasTemplates: boolean;
    searchQuery: string;
    filter: TemplateFilter;
    // The part below the search field and the filter chips.
    list: HTMLElement;
    onAdd: () => void;
    onBack: () => void;
    onSearchInput: (value: string) => void;
    onFilterChange: (filter: TemplateFilter) => void;
  }

  export interface ManageTemplatesListProps {
    templates: UserTemplate[];
    searchQuery: string;
    onRename: (template: UserTemplate) => void;
    onRemove: (template: UserTemplate) => void;
  }

  export interface AddTemplateDialogProps {
    onBrowse: () => Promise<TemplateFileInfo | null>;
    onCancel: () => void;
    // Resolves to false when the template could not be added.
    onAdd: (file: TemplateFileInfo, name: string) => Promise<boolean>;
  }

  export interface AddTemplateDialog {
    element: HTMLElement;
    // Shows a file that was dropped on the window as the file to add.
    setFile: (file: TemplateFileInfo) => void;
  }

  export interface RenameTemplateDialogProps {
    name: string;
    onCancel: () => void;
    onRename: (name: string) => Promise<boolean>;
  }

  export interface RemoveTemplateDialogProps {
    name: string;
    onCancel: () => void;
    onConfirm: () => void;
  }

  function templateTypeLabel(type: TemplateType): string {
    const labels: Record<TemplateType, string> = {
      writer: _('Document template'),
      calc: _('Spreadsheet template'),
      impress: _('Presentation template'),
      draw: _('Drawing template'),
    };
    return labels[type];
  }

  function typeIcon(type: TemplateType): HTMLElement {
    return (
      <span
        class={`backstage-recent-document-icon backstage-doc-icon-${type}`}
        aria-hidden="true"
      />
    );
  }

  function fileNameWithoutExtension(fileName: string): string {
    const dot = fileName.lastIndexOf('.');
    return dot > 0 ? fileName.substring(0, dot) : fileName;
  }

  export function myTemplatesSection(
    props: MyTemplatesSectionProps,
  ): HTMLElement {
    return (
      <>
        <div class="backstage-template-section-header">
          <h4 class="backstage-template-section-title">{_('My templates')}</h4>
          <button
            type="button"
            class="button backstage-manage-templates-button"
            onClick={props.onManageClick}
          >
            {_('Manage')}
          </button>
        </div>
        <div class="backstage-templates-grid backstage-my-templates-grid">
          {addTemplateCard(props.onAddClick)}
          {props.templates.map((template) =>
            templateCard(template, () => props.onTemplateClick(template), {
              favorite: props.favoriteState(template),
            }),
          )}
        </div>
      </>
    );
  }

  function addTemplateCard(onClick: () => void): HTMLElement {
    return (
      <div
        class="backstage-template-card backstage-add-template-card"
        role="button"
        tabindex="0"
        onClick={onClick}
        onKeyDown={(event: KeyboardEvent) => {
          if (event.key === 'Enter' || event.key === ' ') {
            event.preventDefault();
            onClick();
          }
        }}
      >
        <div class="template-thumbnail" aria-hidden="true">
          <span class="backstage-add-template-plus" />
        </div>
        <div class="template-name">{_('Add template')}</div>
      </div>
    );
  }

  export function manageTemplatesView(
    props: ManageTemplatesProps,
  ): HTMLElement {
    return (
      <div class="backstage-manage-templates">
        <div class="backstage-manage-templates-title">
          <button
            type="button"
            class="backstage-manage-templates-back"
            aria-label={_('Back')}
            title={_('Back')}
            onClick={props.onBack}
          >
            <span class="backstage-sidebar-back-icon" aria-hidden="true" />
          </button>
          <h2 class="backstage-content-title">{_('Manage templates')}</h2>
          <button
            type="button"
            class="button backstage-manage-add-template-button"
            onClick={props.onAdd}
          >
            <span class="backstage-add-template-plus" aria-hidden="true" />
            <span>{_('Add template')}</span>
          </button>
        </div>
        <p class="backstage-content-description">
          {props.hasTemplates
            ? _(
                "Rename or remove the templates you added. Built-in templates can't be changed.",
              )
            : _('Templates you add will appear here and in New.')}
        </p>
        {props.hasTemplates ? (
          <>
            <div class="template-search-bar">
              {searchInput(
                props.searchQuery,
                _('Search my templates'),
                props.onSearchInput,
              )}
              {filterChips(props.filter, props.onFilterChange)}
            </div>
            {props.list}
          </>
        ) : (
          <div class="backstage-manage-templates-empty">
            <h3 class="backstage-manage-templates-empty-title">
              {_('No templates yet')}
            </h3>
            <p class="backstage-content-description">
              {_('Add a template to start new documents from your own layout.')}
            </p>
          </div>
        )}
      </div>
    );
  }

  // The table of the templates that match the search and the filter.
  export function manageTemplatesList(
    props: ManageTemplatesListProps,
  ): HTMLElement {
    if (!props.templates.length)
      return (
        <p class="template-grid-empty">
          {props.searchQuery.trim()
            ? _('No templates match your search.')
            : _('No templates of this type.')}
        </p>
      );
    return (
      <table class="backstage-manage-templates-table">
        <thead class="backstage-recent-documents-header">
          <tr class="backstage-recent-documents-header-row">
            <th class="backstage-recent-documents-header-cell">{_('Name')}</th>
            <th class="backstage-recent-documents-header-cell">{_('Type')}</th>
            <th
              class="backstage-recent-documents-header-cell"
              aria-label={_('Actions')}
            />
          </tr>
        </thead>
        <tbody>
          {props.templates.map((template) => (
            <tr
              class="backstage-manage-templates-row"
              id={`backstage-user-template-${template.id}`}
            >
              <td class="backstage-manage-templates-name">
                <div class="backstage-manage-templates-name-wrapper">
                  {typeIcon(template.type)}
                  <span class="backstage-recent-document-name-text">
                    {template.name}
                  </span>
                </div>
              </td>
              <td>
                <span class="backstage-template-type-tag">
                  {typeLabel(template.type)}
                </span>
              </td>
              <td class="backstage-manage-templates-row-actions">
                <button
                  type="button"
                  class="button backstage-rename-template-button"
                  onClick={() => props.onRename(template)}
                >
                  <span
                    class="backstage-template-action-icon"
                    aria-hidden="true"
                  />
                  <span>{_('Rename')}</span>
                </button>
                <button
                  type="button"
                  class="button backstage-remove-template-button"
                  onClick={() => props.onRemove(template)}
                >
                  <span
                    class="backstage-template-action-icon"
                    aria-hidden="true"
                  />
                  <span>{_('Remove')}</span>
                </button>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    );
  }

  export function addTemplateDialog(
    props: AddTemplateDialogProps,
  ): AddTemplateDialog {
    let file: TemplateFileInfo | null = null;
    let fileArea: HTMLElement;
    let errorText: HTMLElement;
    let nameInput: HTMLInputElement;
    let addButton: HTMLButtonElement;

    const updateAddButton = () => {
      addButton.disabled = !file || !nameInput.value.trim();
    };

    const showError = (text: string) => {
      errorText.textContent = text;
    };

    const browse = async () => {
      const picked = await props.onBrowse();
      if (picked) setFile(picked);
    };

    const setFile = (picked: TemplateFileInfo) => {
      if (!picked.type) {
        showError(_('This type of file cannot be added as a template.'));
        return;
      }
      // The name follows the file until the user types a name of their own.
      if (!nameInput.value.trim() || nameInput.value === autoName(file))
        nameInput.value = fileNameWithoutExtension(picked.fileName);
      file = picked;
      showError('');
      fileArea.replaceChildren(fileRow(picked, browse));
      updateAddButton();
    };

    const autoName = (current: TemplateFileInfo | null) =>
      current ? fileNameWithoutExtension(current.fileName) : '';

    const submit = async (event: Event) => {
      event.preventDefault();
      const name = nameInput.value.trim();
      if (!file || !name) return;
      addButton.disabled = true;
      if (!(await props.onAdd(file, name))) {
        showError(_('The template could not be added.'));
        updateAddButton();
      }
    };

    const element = modalShell({
      titleId: 'backstage-add-template-title',
      title: _('Add template'),
      onDismiss: props.onCancel,
      children: (
        <form class="backstage-modal-form" novalidate onSubmit={submit}>
          <div
            class="backstage-template-file-area"
            ref={(element: HTMLElement) => {
              fileArea = element;
            }}
          >
            <div class="backstage-template-drop-zone">
              <span>{_('Drop a file here, or')}</span>
              <button type="button" class="button" onClick={browse}>
                {_('Browse')}
              </button>
            </div>
          </div>
          <p
            class="backstage-modal-error"
            role="alert"
            ref={(element: HTMLElement) => {
              errorText = element;
            }}
          />
          <label class="backstage-modal-field">
            <span class="backstage-modal-field-label">{_('Name')}</span>
            <input
              class="backstage-modal-input ui-edit"
              type="text"
              placeholder={_('Template name')}
              onInput={updateAddButton}
              ref={(element: HTMLInputElement) => {
                nameInput = element;
              }}
            />
          </label>
          <div class="backstage-modal-actions">
            <button type="button" class="button" onClick={props.onCancel}>
              {_('Cancel')}
            </button>
            <button
              type="submit"
              class="button button-primary"
              disabled
              ref={(element: HTMLButtonElement) => {
                addButton = element;
              }}
            >
              {_('Add')}
            </button>
          </div>
        </form>
      ),
    });

    return { element, setFile };
  }

  function fileRow(file: TemplateFileInfo, onReplace: () => void): HTMLElement {
    return (
      <div class="backstage-template-file">
        <div class="backstage-template-file-page" aria-hidden="true">
          {typeIcon(file.type as TemplateType)}
        </div>
        <div class="backstage-template-file-info">
          <div class="backstage-template-file-name">{file.fileName}</div>
          <div class="backstage-template-file-details">
            {templateTypeLabel(file.type as TemplateType) +
              ', ' +
              formatByteSize(file.size)}
          </div>
        </div>
        <button type="button" class="button" onClick={onReplace}>
          {_('Replace')}
        </button>
      </div>
    );
  }

  export function renameTemplateDialog(
    props: RenameTemplateDialogProps,
  ): HTMLElement {
    let nameInput: HTMLInputElement;
    let errorText: HTMLElement;
    const submit = async (event: Event) => {
      event.preventDefault();
      const name = nameInput.value.trim();
      if (!name) nameInput.focus();
      else if (!(await props.onRename(name)))
        errorText.textContent = _('The template could not be renamed.');
    };

    const overlay = modalShell({
      titleId: 'backstage-rename-template-title',
      title: _('Rename template'),
      onDismiss: props.onCancel,
      children: (
        <form class="backstage-modal-form" novalidate onSubmit={submit}>
          <p
            class="backstage-modal-error"
            role="alert"
            ref={(element: HTMLElement) => {
              errorText = element;
            }}
          />
          <label class="backstage-modal-field">
            <span class="backstage-modal-field-label">{_('Name')}</span>
            <input
              class="backstage-modal-input ui-edit"
              type="text"
              value={props.name}
              ref={(element: HTMLInputElement) => {
                nameInput = element;
              }}
            />
          </label>
          <div class="backstage-modal-actions">
            <button type="button" class="button" onClick={props.onCancel}>
              {_('Cancel')}
            </button>
            <button type="submit" class="button button-primary">
              {_('Rename')}
            </button>
          </div>
        </form>
      ),
    });

    app.timerRegistry.setTimeout(
      'backstagerenameselect',
      () => nameInput?.select(),
      0,
    );
    return overlay;
  }

  export function removeTemplateDialog(
    props: RemoveTemplateDialogProps,
  ): HTMLElement {
    const overlay = modalShell({
      titleId: 'backstage-remove-template-title',
      title: _('Remove this template?'),
      onDismiss: props.onCancel,
      children: (
        <>
          <p class="backstage-modal-subtitle">
            {_('"%1" will be removed from My templates.').replace(
              '%1',
              props.name,
            )}
          </p>
          {removeActions({
            onCancel: props.onCancel,
            onConfirm: props.onConfirm,
          })}
        </>
      ),
    });
    return overlay;
  }
}
