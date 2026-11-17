// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

function isCtrlKey (e: KeyboardEvent) {
    if (window.detect(window.features.apple))
        return e.metaKey;
    else
        return e.ctrlKey;
}

function isMacCtrlKey (e: KeyboardEvent) {
    if (window.detect(window.features.apple))
        return e.ctrlKey;
    else
        return false;
}

enum Mod {
    NONE    = 0,
    CTRL    = 1,
    ALT     = 2,
    SHIFT   = 4,
    MACCTRL = 8, // Ctrl (*not Cmd*) on a Mac
}

enum ViewType {
    Edit = 0,
    ReadOnly = 1,
}

type shortcutCallback = () => void;

class ShortcutDescriptor {
    docType: string; // if undefined then all apps match
    eventType: string | readonly string[];
    modifier: Mod;
    keyCode: number | readonly number[] | null;
    key: string | null;
    unoAction: string;
    dispatchAction: string;
    dispatchData: any;
    viewType: ViewType;
    preventDefault: boolean;
    features: Platform;
    showViewModeAttention: boolean;
    condition: (() => boolean) | null;

    constructor({
        docType = null,
        eventType,
        modifier = Mod.NONE,
        keyCode = null,
        key = null,
        unoAction = null,
        dispatchAction = null,
        dispatchData = null,
        viewType = null,
        preventDefault = true,
        features = Platform.ALL,
        showViewModeAttention = true,
        condition = null,
    }: {
        /** The type of document to register this keybind in. If omitted, the keybind will be registered for all document types */
        docType?: 'text' | 'presentation' | 'drawing' | 'spreadsheet',
        /** The event type or types to register this keybind for. Generally you probably want this to be 'keydown' */
        eventType: string | readonly string[],
        /** A bitfield of modifiers you want to be active. For example, Mod.CTRL | Mod.SHIFT would mean that *both* control and shift would need to be held while pressing the keybind.

        On Mac, command is seen as Mod.CTRL and there is a separate Mod.MACCTRL to read control

        If omitted, no modifier will be required

        @default Mod.NONE */
        modifier?: Mod,
        /** The keyCode of the shortcut trigger key, as seen in the table on https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/keyCode

        It is rare but possible for this to provide different results in different browsers. For example, Safari sometimes returns keyCode 0 (unknown) on some event types when the bind would trigger certain system shortcuts. You should test any shortcuts you make with 'keyCode' on different browsers and systems to make sure they all work as intended.

        You must provide at least one of 'key' or 'keyCode'. If you provide both, either a matching key or a matching keyCode will trigger the binding.

        @deprecated Unless you know you need this, you should probably use 'key' instead, as this relies on a deprecated web API. It will not be removed as dead keys cannot be properly handled in the key API */
        keyCode?: number | readonly number[],
        /** The key of the shortcut trigger key, as seen on https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/key

        You must provide at least one of 'key' or 'keyCode'. If you provide both, either a matching key or a matching keyCode will trigger the binding.

        When adding shortcuts using 'key' you should always test in multiple browsers and systems or ask for review from someone who can. Sometimes, particularly on keybinds that trigger "dead keys" (accents for letters), there are inconsistencies in different browsers. It's also possible for multiple different key combinations to trigger the same typed glyph - leading to this munging different keybinds together */
        key?: string,
        /** The uno command, including its .uno: prefix, to run when this keybind is pressed

        If both the unoAction and dispatchAction are provided, only the unoAction will trigger. The dispatchAction will be ignored.

        If omitted, no uno command will be run when this keybind is pressed */
        unoAction?: string,
        /** The action to dispatch when the keybind is pressed

        If both the unoAction and dispatchAction are provided, only the unoAction will trigger. The dispatchAction will be ignored.

        If omitted, no action will be dispatched when this keybind is pressed */
        dispatchAction?: string,
        /** The optional data to pass to the dispatcher if dispatchAction is used*/
        dispatchData?: any,
        /** The view type (Edit or ReadOnly) to restrict this keybind to

        If ommitted, the keybind will be active in both Edit and ReadOnly view types */
        viewType?: ViewType,
        /** Whether to prevent the default system binding. This is important if you're overriding a system binding, and not generally harmful otherwise

        Note that all keypresses which include a control (or a command on mac) are already preventDefaulted elsewhere to be sent to core. If you want to stop this, you should manually register a binding without any action but with this option set to false

        Note that some browsers may not allow you to preventDefault some events. For example, Safari will not allow you to preventDefault the command+r keybind to refresh the current tab

        @default true
        */
        preventDefault?: boolean,
        /** A bitfield of features you want this keybind to be active on

        A user with any platform where the feature is enabled will get the keybind, so providing `window.features.app | window.features.online` would bind the keybind for all app or online users (but not emscripten).

        If omitted, the keybind will be active everywhere

        Look at how other keybinds determine this value, as it's easy to leave out platforms unintentionally.
        For example, if you try to do platform detection with `Platform.WINDOWS | Platform.LINUX` you would be leaving out all of the desktop and emscripten apps - you would probably want `~window.features.apple` instead
        */
        features?: Platform,
        /** Whether pressing this shortcut in read-only (viewing) mode shows
        the "you are in view mode" attention animation and is otherwise
        swallowed. True for shortcuts that send a uno command which could
        modify the document. Set it to false for read-only safe uno commands
        (for example navigation) that should still run while viewing.

        @default true */
        showViewModeAttention?: boolean,
        /** An optional predicate evaluated when the key combination otherwise
        matches. Return false to make this shortcut not match, so processing
        falls through to any other handler for the same key.

        If omitted, the shortcut always matches when its key/modifier match. */
        condition?: () => boolean,
    }) {
        app.console.assert(keyCode !== null || key !== null, 'registering a keyboard shortcut without specifying either a key or a keyCode - this will result in an untriggerable shortcut');

        // Layout independence: a single letter or digit key always has a
        // fixed keyCode (the US-layout ASCII value), even though event.key
        // changes by layout (physical F is event.key === 'а' on Russian).
        // Auto-deriving keyCode from key here means a binding with just
        // key:'f' still fires on the physical F key everywhere.
        if (keyCode === null && key !== null && key.length === 1) {
            const code = key.charCodeAt(0);
            if (code >= 0x41 && code <= 0x5A) keyCode = code;            // A-Z
            else if (code >= 0x61 && code <= 0x7A) keyCode = code - 0x20; // a-z
            else if (code >= 0x30 && code <= 0x39) keyCode = code;        // 0-9
        }

        this.docType = docType;
        this.eventType = eventType;
        this.modifier = modifier;
        this.keyCode = keyCode;
        this.key = key;
        this.unoAction = unoAction;
        this.dispatchAction = dispatchAction;
        this.dispatchData = dispatchData;
        this.viewType = viewType;
        this.preventDefault = preventDefault;
        this.features = features;
        this.showViewModeAttention = showViewModeAttention;
        this.condition = condition;
    }
}

// The engine writes a shortcut as its modifiers and key name joined by '+', as in 'Ctrl+Shift+B',
// 'Ctrl+Shift++' or 'Shift+Return'. Returns the modifier and the key as KeyboardEvent.key names it.
function parseEngineShortcut(text: string): { modifier: Mod, key: string } | null {
    const engineKeyNames: Record<string, string> = {
        Return: 'Enter', Space: ' ', Up: 'ArrowUp', Down: 'ArrowDown', Left: 'ArrowLeft',
        Right: 'ArrowRight', Esc: 'Escape', Del: 'Delete', Ins: 'Insert',
    };
    const parts = text.endsWith('++') ? text.slice(0, -2).split('+').concat(['+']) : text.split('+');
    const name = parts.pop();
    if (!name) return null;
    let modifier = Mod.NONE;
    for (const part of parts) {
        if (part === 'Ctrl') modifier |= Mod.CTRL;
        else if (part === 'Alt') modifier |= Mod.ALT;
        else if (part === 'Shift') modifier |= Mod.SHIFT;
        else return null;
    }
    return { modifier: modifier, key: engineKeyNames[name] || name };
}

class KeyboardShortcuts {
    map: any;
    definitions: Map<string, Array<ShortcutDescriptor>>;

    constructor() {
        this.definitions = new Map<string, Array<ShortcutDescriptor>>();
    }

    private findShortcut(language: string, eventType: string, modifier: Mod, keyCode: number | undefined, key: string | undefined, features: Platform | null = null)
        : ShortcutDescriptor | undefined {
        const descriptors = this.definitions.get(language);
        if (!descriptors) {
            return undefined;
        }

        const docType = this.map._docLayer ? this.map._docLayer._docType : '';
        const viewType = this.map.isEditMode() ? ViewType.Edit : ViewType.ReadOnly;

        const shortcuts = descriptors.filter((descriptor: ShortcutDescriptor) => {
            const keyMatches = descriptor.key === key;
            const keyCodeMatches = Array.isArray(descriptor.keyCode) ? descriptor.keyCode.includes(keyCode) : descriptor.keyCode === keyCode;

            return (!descriptor.docType || descriptor.docType === docType) &&
                (Array.isArray(descriptor.eventType) ? descriptor.eventType.includes(eventType) : descriptor.eventType === eventType) &&
                descriptor.modifier === modifier &&
                (descriptor.viewType === null || descriptor.viewType === viewType) &&
                window.detect(descriptor.features, features) &&
                (keyMatches || keyCodeMatches) &&
                (!descriptor.condition || descriptor.condition());
        });

        if (shortcuts.length > 1) {
            throw 'Multiple definitions of the same keyboard shortcut';
        }

        if (shortcuts.length) {
            return shortcuts[0];
        }

        return undefined;
    }

    /// returns true if handled action
    private processEventImpl(language: string, event: KeyboardEvent) : boolean {
        const eventType = event.type;
        const ctrl = isCtrlKey(event);
        const shift = event.shiftKey;
        const alt = event.altKey;
        const keyCode = event.which;
        const key = event.key;
        const macctrl = isMacCtrlKey(event);
        const modifier = (ctrl ? Mod.CTRL : Mod.NONE) |
            (shift ? Mod.SHIFT : Mod.NONE) |
            (alt ? Mod.ALT : Mod.NONE) |
            (macctrl ? Mod.MACCTRL : Mod.NONE);

        const shortcut = this.findShortcut(language, eventType, modifier, keyCode, key);

        if (shortcut) {
            // In read-only mode, block shortcuts that send uno commands to core, or
            // that run an extension's own command (an opaque script we cannot
            // otherwise tell apart from one that edits the document), unless they
            // are explicitly meant for read-only use.  (Extensions are hidden entirely by
            // Control.Extension.ts's loadExtensions when a doc opens in a restricted mode, so the
            // ext: branch here only fires for a doc that was editable at open and has since been
            // toggled into view mode.)
            const isExtensionAction =
                !!shortcut.dispatchAction && shortcut.dispatchAction.startsWith('ext:');

            if (!this.map.isEditMode() && (shortcut.unoAction || isExtensionAction) &&
                shortcut.showViewModeAttention &&
                shortcut.viewType !== ViewType.ReadOnly) {
                event.preventDefault();
                this.map.uiManager.showViewModeAttention();

                return true;
            }

            let action = 'disabled';
            if (shortcut.unoAction) {
                action = shortcut.unoAction;
                this.map.sendUnoCommand(action);
            } else if (shortcut.dispatchAction) {
                action = shortcut.dispatchAction;
                app.dispatcher.dispatch(action, shortcut.dispatchData);
            }

            if (shortcut.preventDefault) {
                event.preventDefault();
            }

            console.debug('handled keyboard shortcut: ' + action);
            return true;
        }

        return false;
    }

    // has to be called before use
    public initialize(map: any) {
        this.map = map;

        // in cypress it can fail on load to not allow for duplicated shortcuts
        if (window.L.Browser.cypressTest) {
            this.map.on('docloaded', () => { keyboardShortcuts.verifyShortcuts(); });
        }
    }

    public processEvent(language: string, event: KeyboardEvent) : boolean {
        if (!this.map) {
            throw 'KeyboardShortcuts not initialized';
        }

        // The language comes from the URL, so it can carry a region -
        // 'de-DE' rather than 'de'.  Only a few of those have a table of
        // their own ('pt-BR'); the rest use their base language's, the way
        // the shortcut tooltips already do.
        const dash = language.indexOf('-');
        if (dash !== -1 && !this.definitions.has(language)) {
            language = language.substring(0, dash);
        }

        if (this.processEventImpl(language, event)) {
            return true;
        }

        return this.processEventImpl('default', event);
    }

    // True if a shortcut the editor already has would match a keydown with this exact modifier and
    // key, for this document's own docType or for every docType, on this platform. That covers the
    // descriptors of the 'default' list and of the user's own language, which is tried first, and
    // the shortcuts of the engine's commands. Each keypress may match one descriptor at most, and a
    // descriptor that matches a keypress keeps it from reaching the engine.
    private collidesWithExisting(docType: string, modifier: Mod, key: string): boolean {
        const candidate = new ShortcutDescriptor({ eventType: 'keydown', modifier: modifier, key: key });
        const sameKey = (d: ShortcutDescriptor): boolean => {
            const keyCodes = Array.isArray(d.keyCode) ? d.keyCode : [d.keyCode];
            return (d.key !== null && d.key.toLowerCase() === key.toLowerCase()) ||
                (candidate.keyCode !== null && keyCodes.indexOf(candidate.keyCode) !== -1);
        };
        const language = app.UI && app.UI.language ? app.UI.language.fromURL : undefined;
        const descriptors = (this.definitions.get('default') || []).concat(
            language && language !== 'default' ? this.definitions.get(language) || [] : []);
        const inEditor = descriptors.some((d) => {
            const eventTypes = Array.isArray(d.eventType) ? d.eventType : [d.eventType];
            return eventTypes.indexOf('keydown') !== -1 &&
                d.modifier === modifier &&
                sameKey(d) &&
                (!d.docType || d.docType === docType) &&
                window.detect(d.features);
        });
        if (inEditor) return true;
        if (typeof unoShortcutsMap === 'undefined') return false;
        return Object.keys(unoShortcutsMap).some((command) => {
            const engine = parseEngineShortcut(unoShortcutsMap[command]);
            return engine !== null && engine.modifier === modifier &&
                engine.key.toLowerCase() === key.toLowerCase();
        });
    }

    // Registers each loaded extension's contributes.keybindings entries as
    // dispatchAction-based shortcuts in the 'default' language list. Called once,
    // after extension discovery resolves (see ServerConnectionService's
    // onDocumentLoaded) - there is no later point that needs to call this again,
    // since the extension list itself is only ever populated once per document.
    public applyExtensionKeybindings(): void {
        if (!window.enableExperimentalFeatures) return;
        const exts = (this.map && this.map._extensions) || {};
        const docType = this.map && this.map._docLayer ? this.map._docLayer._docType : '';
        const defaultShortcuts = this.definitions.get('default');

        Object.keys(exts).sort().forEach((extId: string) => {
            const manifest = exts[extId].options.manifest;
            const contributed = manifest.contributes && manifest.contributes.keybindings;
            if (!contributed || !contributed.length) return;
            const commands = manifest.contributes.commands || [];

            contributed.forEach((entry: any) => {
                const command = commands.filter((c: any) => c.id === entry.command)[0];
                if (!command) {
                    console.warn('extension ' + extId + ': keybinding references unknown command "' + entry.command + '"');
                    return;
                }

                const modifierNames: string[] = entry.modifier || [];
                let modifier = Mod.NONE;
                if (modifierNames.indexOf('ctrl') !== -1) modifier |= Mod.CTRL;
                if (modifierNames.indexOf('alt') !== -1) modifier |= Mod.ALT;
                if (modifierNames.indexOf('shift') !== -1) modifier |= Mod.SHIFT;

                // Any key without ctrl or alt belongs to the document: a letter is typed, and Enter,
                // Tab, Delete or an F key already has a meaning there.
                const key: string = entry.key;
                if (!(modifier & (Mod.CTRL | Mod.ALT))) {
                    console.warn('extension ' + extId + ': keybinding for "' + entry.command +
                        '" ("' + key + '") needs ctrl or alt, not shift alone or no modifier');
                    return;
                }

                if (this.collidesWithExisting(docType, modifier, key)) {
                    console.warn('extension ' + extId + ': keybinding for "' + entry.command +
                        '" ("' + key + '") collides with an existing shortcut and was not registered');
                    return;
                }

                defaultShortcuts.push(new ShortcutDescriptor({
                    eventType: 'keydown',
                    modifier: modifier,
                    key: key,
                    dispatchAction: 'ext:' + extId + ':' + entry.command,
                }));
            });
        });
    }

    public verifyShortcuts() : void {
        console.debug('KeyboardShortcuts.verifyShortcuts start');
        this.definitions.forEach((shortcuts, language) => {
            shortcuts.forEach((shortcut) => {
                // throws an exception if finds duplicated
                const shortcutEventTypes = Array.isArray(shortcut.eventType) ? shortcut.eventType : [shortcut.eventType];
                const shortcutKeyCodes = Array.isArray(shortcut.keyCode) ? shortcut.keyCode : [shortcut.keyCode];

                for (const eventType of shortcutEventTypes) {
                    for (const keyCode of shortcutKeyCodes) {
                        if (keyCode === null) {
                            continue;
                        }

                        this.findShortcut(language,
                            eventType, shortcut.modifier, keyCode, undefined, shortcut.features);
                    }

                    if (shortcut.key === null) {
                        continue;
                    }

                    this.findShortcut(language,
                        eventType, shortcut.modifier, undefined, shortcut.key, shortcut.features);
                }
            });
        });
        console.debug('KeyboardShortcuts.verifyShortcuts finished');
    }
}

const keyboardShortcuts = new KeyboardShortcuts();

// Default shortcuts.
keyboardShortcuts.definitions.set('default', new Array<ShortcutDescriptor>(

    // FIXME: Should we mark shortcuts that are supposed to work *only* in editing mode with
    // viewType: ViewType.Edit? Having viewType as null means the shortcut is supposed to work in
    // either viewing or editing mode, and having it as ViewType.ReadOnly means it is supposed to
    // work only in viewing mode. At least if you believe the comment for viewType in the
    // ShortcutDescriptor constructor.

    // All document types.
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'o', features: window.features.fileLocalShortcuts, unoAction: '.uno:Open' }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'w', features: window.features.app & window.features.multiWindow & ~window.features.apple, unoAction: '.uno:CloseWin', showViewModeAttention: false }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'PageDown', features: window.features.tabsKeybinds, unoAction: '.uno:NextDocumentTab', showViewModeAttention: false }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'PageUp', features: window.features.tabsKeybinds, unoAction: '.uno:PreviousDocumentTab', showViewModeAttention: false }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'Tab', features: window.features.tabsKeybinds, unoAction: '.uno:RecentlyUsedDocumentTab', showViewModeAttention: false }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'E', dispatchAction: 'switchtoedit', viewType: ViewType.ReadOnly, showViewModeAttention: false }),

    /*
        Disable F5 or assign it something to prevent browser refresh.
        Disable multi-sheet selection shortcuts in Calc.
        Disable F2 in Writer, formula bar is unsupported, and messes with further input.
        Disable CTRL+SHIFT+N because core side template dialog is not supported on Online.
    */
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'b', unoAction: '.uno:Bold' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'i', unoAction: '.uno:Italic' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'u', unoAction: '.uno:Underline' }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'N' }),
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F1', dispatchAction: 'showhelp' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.ALT, key: 'F1', dispatchAction: 'focustonotebookbar' }),
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F6', dispatchAction: 'focusnextregion', showViewModeAttention: false }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.SHIFT, key: 'F6', dispatchAction: 'focuspreviousregion', showViewModeAttention: false }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'f', dispatchAction: 'home-search' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'p', dispatchAction: 'print' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 's', dispatchAction: 'save', dispatchData: 'keyboard' }),
    // On a Mac, Cmd+S saves and Ctrl+S does nothing. Every save goes through the save action
    // above, which also uploads the file to storage and checks it for conflicting changes.
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.MACCTRL, key: 's', features: window.features.apple }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT | Mod.ALT, key: 'V', unoAction: '.uno:PasteSpecial', features: window.features.browser }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'V', dispatchAction: '.uno:PasteSpecial', features: window.features.app }),

    // Calc.
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'PageUp', dispatchAction: 'scrollpageup', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'PageDown', dispatchAction: 'scrollpagedown', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageUp', features: Platform.APP_LINUX, unoAction: '.uno:JumpToPrevTable', showViewModeAttention: false }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageDown', features: Platform.APP_LINUX, unoAction: '.uno:JumpToNextTable', showViewModeAttention: false }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageUp', features: ~Platform.APP_LINUX }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageDown', features: ~Platform.APP_LINUX }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F5' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F9', unoAction: '.uno:Calculate' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL, key: ',', unoAction: '.uno:InsertCurrentDate' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL, key: ';', unoAction: '.uno:InsertCurrentDate' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F10', dispatchAction: 'sheettabmenu',
        condition: () => { const el = document.activeElement; return !!el && el.classList.contains('spreadsheet-tab'); } }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F10', dispatchAction: 'rowcolumnheadermenu',
        condition: () => {
            const el = document.activeElement;
            if (el && el.classList.contains('spreadsheet-tab'))
                return false;
            const map = app.map as any;
            return !!(map && (map.wholeColumnSelected || map.wholeRowSelected));
        } }),

    // Writer.
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'PageUp', dispatchAction: 'scrollpageup', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'PageDown', dispatchAction: 'scrollpagedown', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F2' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F3', unoAction: '.uno:ExpandGlossary' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', modifier: Mod.CTRL, key: 'F3' }),
    // browsers fire no keypress for function keys
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F4',
        unoAction: '.uno:GraphicDialog' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F4',
        unoAction: '.uno:JumpToNextFrame' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F5' }),

    // Impress.
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'F5', dispatchAction: 'presentation' }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'PageUp', dispatchAction: 'previouspart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'PageDown', dispatchAction: 'nextpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', modifier: Mod.CTRL, key: 'a', dispatchAction: 'selectallslides',
        condition: () => app.map.keyboard._slideSorterFocused() }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F3', unoAction: '.uno:CopyObjects', viewType: ViewType.Edit }),

    // Draw.
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'F5' }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'PageUp', dispatchAction: 'previouspart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'PageDown', dispatchAction: 'nextpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F3', unoAction: '.uno:CopyObjects', viewType: ViewType.Edit }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'End', dispatchAction: 'lastpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'Home', dispatchAction: 'firstpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', modifier: Mod.CTRL, key: 'a', dispatchAction: 'selectallslides',
        condition: () => app.map.keyboard._slideSorterFocused() }),


    // Prevent F7 from triggering Caret Browsing in apps.
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F7', unoAction: '.uno:SpellingAndGrammarDialog', features: window.features.app }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.ALT | Mod.CTRL, key: 'p', dispatchAction: 'userlist' }),

    // Passthrough some system shortcuts
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'I', preventDefault: false, features: ~window.features.app & ~window.features.apple }), // Open browser developer tools on Non-MacOS - shift means the I here is capital
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.ALT, keyCode: 73 /* keyCode('I') === 73 */, preventDefault: false, features: ~window.features.app & window.features.apple }), // Open browser developer tools on MacOS - registered with keyCode as alt+i triggers a dead key on MacOS
    new ShortcutDescriptor({ eventType: ['keydown', 'keypress'], modifier: Mod.CTRL | Mod.MACCTRL, key: ' ', preventDefault: false, features: window.features.apple }), // On MacOS, open system emoji picker - bound to keypress as well as keydown since as that is needed on webkit browsers (such as Safari or Orion)
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'r', preventDefault: false, features: window.features.browser & window.features.apple }), // Refresh browser tab
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'R', preventDefault: false, features: window.features.browser & ~window.features.apple }), // Refresh browser tab & clear cache
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'm', preventDefault: false, features: window.features.apple }), // On MacOS, minimize window
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'q', preventDefault: false, features: window.features.apple }), // On MacOS, quit browser
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'w', features: window.features.browser | window.features.apple, preventDefault: false }), // Close current tab
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'n', dispatchAction: 'backstage-new', features: window.features.backstage }), // Open backstage "New" tab in desktop apps
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'n', preventDefault: false, features: window.features.browser }), // Open new browser window
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 't', preventDefault: false }), // Open new browser tab
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: '`', preventDefault: false, features: window.features.apple }), // Cycle through windows
));

// German shortcuts: only online-specific dispatchAction entries and
// passthrough shortcuts.  A shortcut that is a UNO command belongs in
// core's Accelerators.xcu, which the kit reads in the view's own
// language.
keyboardShortcuts.definitions.set('de', new Array<ShortcutDescriptor>(
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F12', dispatchAction: 'saveas' }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F5', dispatchAction: 'presentation' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', modifier: Mod.ALT | Mod.CTRL, key: 's', dispatchAction: 'home-search' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F2', dispatchAction: 'insertcomment' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F4', dispatchAction: 'togglerelative' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F5', dispatchAction: 'focusonaddressinput' }),
    // Passthrough some system shortcuts
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: '`', preventDefault: false, features: window.features.apple }), // Cycle through windows
));

// Danish shortcuts: same rule as for German
keyboardShortcuts.definitions.set('da', new Array<ShortcutDescriptor>(
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'b', dispatchAction: 'home-search' }),
));

window.KeyboardShortcuts = keyboardShortcuts;
