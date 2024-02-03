/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

if (!('preRun' in Module)) Module['preRun'] = [];
Module.preRun.push(function() {
    // Wait until Embind is set up:
    Module.addOnPreMain(function() {
        Module.registerType(Module.getOUStringRawType(), {
            name: 'rtl::OUString',
            'fromWireType'(ptr) {
                let data = Module.HEAPU32[ptr >> 2];
                let length = Module.HEAPU32[(data >> 2) + 1];
                let buffer = data + 8;
                let str = '';
                for (let i = 0; i < length; ++i) {
                    let c = Module.HEAPU16[(buffer >> 1) + i];
                    str += String.fromCharCode(c);
                }
                Module.rtl_uString_release(data);
                Module._free(ptr);
                return str;
            },
            'toWireType'(destructors, value) {
                if (typeof value != 'string') {
                    Module.throwBindingError('Cannot pass non-string to C++ OUString');
                }
                let data = Module._malloc(8 + (value.length + 1) * 2);
                Module.HEAPU32[data >> 2] = 1;
                Module.HEAPU32[(data >> 2) + 1] = value.length;
                let buffer = data + 8;
                for (let i = 0; i < value.length; ++i) {
                    Module.HEAPU16[(buffer >> 1) + i] = value.charCodeAt(i);
                }
                Module.HEAPU16[(buffer >> 1) + value.length] = 0;
                let ptr = Module._malloc(4);
                Module.HEAPU32[ptr >> 2] = data;
                if (destructors !== null) {
                    destructors.push(Module._free, ptr);
                }
                return ptr;
            },
            'argPackAdvance': 8,
            'readValueFromPointer'(pointer) {
                return this['fromWireType'](Module.HEAPU32[((pointer)>>2)]);
            },
            destructorFunction(ptr) {
                Module._free(ptr);
            },
        });
    });
});
