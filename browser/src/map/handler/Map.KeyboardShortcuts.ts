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

/* Surely these should be in some common .ts file where all similar lines would be centralised? */
declare var ThisIsTheMacOSApp: any;
declare var ThisIsTheQtApp: any;
declare var ThisIsTheWindowsApp: any;
declare var unoShortcutsL10NKeyBindings: any;

function isCtrlKey (e: KeyboardEvent) {
    if ((window as any).ThisIsTheiOSApp || window.L.Browser.mac)
        return e.metaKey;
    else
        return e.ctrlKey;
}

function isMacCtrlKey (e: KeyboardEvent) {
    if ((window as any).ThisIsTheiOSApp || window.L.Browser.mac)
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

enum Platform {
    ANDROIDAPP  = 1,
    IOSAPP      = 2,
    MAC         = 4, // Browser on macOS
    WINDOWS     = 8, // Browser on Windows
    LINUX       = 16, // Actually means "none of the others", presumably browser on Linux
    CHROMEOSAPP = 32,
    CODAWINDOWS = 64,
    CODAMAC     = 128,
    CODAQT      = 256,
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
    platform: Platform;

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
        platform = null,
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
        /** A bitfield of platforms you want this keybind to be active on

        A user on any provided platform will get the keybind, so providing Platform.WINDOWS | Platform.LINUX would bind the key for both Windows *and* Linux (but not iOS, Android or Chromebook).

        Platforms are detected in a mutually-exclusive fashion - that is: although, for example, Chromebook app users are necessarily using the Android app, they will only ever be registered as their most specific possible platform.

        There will never be a platformless user. If a platform can't be detected, the user will be assumed to be using Linux. This is probably an OK assumption, given Windows and MacOS already have specific detections earlier.

        If ommitted, the keybind will be active on all platforms
        */
        platform?: Platform,
    }) {
        app.console.assert(keyCode !== null || key !== null, 'registering a keyboard shortcut without specifying either a key or a keyCode - this will result in an untriggerable shortcut');

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
        this.platform = platform;
    }
}

class KeyboardShortcuts {
    map: any;
    definitions: Map<string, Array<ShortcutDescriptor>>;

    constructor() {
        this.definitions = new Map<string, Array<ShortcutDescriptor>>();
    }

    private findShortcut(language: string, eventType: string, modifier: Mod, keyCode: number | undefined, key: string | undefined, platform: Platform)
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
                (!descriptor.platform || (descriptor.platform & platform)) &&
                (keyMatches || keyCodeMatches);
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
        const platform = window.mode.isChromebook() ? Platform.CHROMEOSAPP :
                         window.ThisIsTheAndroidApp ? Platform.ANDROIDAPP : // Cannot come before window.mode.isChromebook() as all Chromebook app users are necessarily also Android app users
                         window.ThisIsTheiOSApp ? Platform.IOSAPP :
                         window.ThisIsTheWindowsApp ? Platform.CODAWINDOWS :
                         window.ThisIsTheMacOSApp ? Platform.CODAMAC :
                         window.ThisIsTheQtApp ? Platform.CODAQT :
                         window.L.Browser.mac ? Platform.MAC :
                         window.L.Browser.win ? Platform.WINDOWS :
                         Platform.LINUX;

        const shortcut = this.findShortcut(language, eventType, modifier, keyCode, key, platform);

        if (shortcut) {
            // In read-only mode, block shortcuts that send uno commands
            // to core unless they are explicitly meant for read-only use.

            // FIXME: For some reason we need to check for .uno:CloseWin separately here. That is
            // supposed to work both in viewing mode (called ViewType.ReadOnly) and editing mode.

            if (!this.map.isEditMode() && shortcut.unoAction &&
                shortcut.unoAction !== '.uno:CloseWin' &&
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

        if (this.processEventImpl(language, event)) {
            return true;
        }

        return this.processEventImpl('default', event);
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
                            eventType, shortcut.modifier, keyCode, undefined, shortcut.platform);
                    }

                    if (shortcut.key === null) {
                        continue;
                    }

                    this.findShortcut(language,
                        eventType, shortcut.modifier, undefined, shortcut.key, shortcut.platform);
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
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'o', platform: Platform.CODAWINDOWS | Platform.CODAMAC | Platform.CODAQT, unoAction: '.uno:Open' }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'w', platform: Platform.CODAWINDOWS | Platform.CODAQT, unoAction: '.uno:CloseWin' }),

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
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'f', dispatchAction: 'home-search' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'p', dispatchAction: 'print' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 's', dispatchAction: 'save', dispatchData: 'keyboard' }),
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT | Mod.ALT, key: 'V', unoAction: '.uno:PasteSpecial', platform: Platform.WINDOWS | Platform.LINUX | Platform.MAC | Platform.CHROMEOSAPP}),

    // Calc.
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageUp' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'PageDown' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F5' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL, key: ',', unoAction: '.uno:InsertCurrentDate' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.CTRL, key: ';', unoAction: '.uno:InsertCurrentDate' }),

    // Writer.
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F2' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F3', unoAction: '.uno:ExpandGlossary' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', modifier: Mod.CTRL, key: 'F3' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', key: 'F5' }),

    // Impress.
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'F5', dispatchAction: 'presentation' }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'PageUp', dispatchAction: 'previouspart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', key: 'PageDown', dispatchAction: 'nextpart', viewType: ViewType.ReadOnly }),

    // Draw.
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'F5' }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'PageUp', dispatchAction: 'previouspart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'PageDown', dispatchAction: 'nextpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'End', dispatchAction: 'lastpart', viewType: ViewType.ReadOnly }),
    new ShortcutDescriptor({ docType: 'drawing', eventType: 'keydown', key: 'Home', dispatchAction: 'firstpart', viewType: ViewType.ReadOnly }),


    // Prevent F7 from triggering Caret Browsing in desktop apps.
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F7', unoAction: '.uno:SpellingAndGrammarDialog', platform: Platform.CODAWINDOWS | Platform.CODAMAC | Platform.CODAQT }),

    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.ALT | Mod.CTRL, key: 'p', dispatchAction: 'userlist' }),

    // Passthrough some system shortcuts
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'I', preventDefault: false, platform: Platform.WINDOWS | Platform.LINUX }), // Open browser developer tools on Non-MacOS - shift means the I here is capital
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.ALT, keyCode: 73 /* keyCode('I') === 73 */, preventDefault: false, platform: Platform.MAC }), // Open browser developer tools on MacOS - registered with keyCode as alt+i triggers a dead key on MacOS
    new ShortcutDescriptor({ eventType: ['keydown', 'keypress'], modifier: Mod.CTRL | Mod.MACCTRL, key: ' ', preventDefault: false, platform: Platform.MAC | Platform.IOSAPP }), // On MacOS, open system emoji picker - bound to keypress as well as keydown since as that is needed on webkit browsers (such as Safari or Orion)
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'r', preventDefault: false, platform: Platform.MAC }), // Refresh browser tab
	new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: 'R', preventDefault: false, platform: Platform.WINDOWS | Platform.LINUX }), // Refresh browser tab & clear cache
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'm', preventDefault: false, platform: Platform.MAC }), // On MacOS, minimize window
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'q', preventDefault: false, platform: Platform.MAC }), // On MacOS, quit browser
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'w', platform: Platform.LINUX | Platform.WINDOWS | Platform.MAC, preventDefault: false }), // Close current tab
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 'n', preventDefault: false }), // Open new browser window
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: 't', preventDefault: false }), // Open new browser tab
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL, key: '`', preventDefault: false, platform: Platform.MAC }), // Cycle through windows
));

// German shortcuts: only online-specific dispatchAction entries and
// passthrough shortcuts.  UNO command bindings are generated from
// Accelerators.xcu (see below).
keyboardShortcuts.definitions.set('de', new Array<ShortcutDescriptor>(
    new ShortcutDescriptor({ eventType: 'keydown', key: 'F12', dispatchAction: 'saveas' }),
    new ShortcutDescriptor({ docType: 'presentation', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F5', dispatchAction: 'presentation' }),
    new ShortcutDescriptor({ docType: 'text', eventType: 'keydown', modifier: Mod.ALT | Mod.CTRL, key: 's', dispatchAction: 'home-search' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', modifier: Mod.SHIFT, key: 'F2', dispatchAction: 'insertcomment' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F4', dispatchAction: 'togglerelative' }),
    new ShortcutDescriptor({ docType: 'spreadsheet', eventType: 'keydown', key: 'F5', dispatchAction: 'focusonaddressinput' }),
    // Passthrough some system shortcuts
    new ShortcutDescriptor({ eventType: 'keydown', modifier: Mod.CTRL | Mod.SHIFT, key: '`', preventDefault: false, platform: Platform.MAC }), // Cycle through windows
));

// Register per-language keyboard shortcuts generated from core's
// Accelerators.xcu by scripts/unoshortcuts.py.
if (typeof unoShortcutsL10NKeyBindings !== 'undefined') {
    for (const [lang, bindings] of Object.entries(unoShortcutsL10NKeyBindings)) {
        let existing = keyboardShortcuts.definitions.get(lang);
        if (!existing) {
            existing = new Array<ShortcutDescriptor>();
            keyboardShortcuts.definitions.set(lang, existing);
        }

        // Track key+modifier combos already defined manually so we
        // don't create duplicates (which would throw in findShortcut).
        const existingKeys = new Set<string>();
        for (const d of existing) {
            existingKeys.add(d.key + '|' + d.modifier);
        }

        for (const b of bindings as any[]) {
            const k = b.key + '|' + b.modifier;
            if (!existingKeys.has(k)) {
                existing.push(new ShortcutDescriptor({
                    eventType: 'keydown',
                    key: b.key,
                    modifier: b.modifier,
                    unoAction: b.unoAction,
                    docType: b.docType,
                }));
                existingKeys.add(k);
            }
        }
    }
}

window.KeyboardShortcuts = keyboardShortcuts;
