/* -*- Mode: JS; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

Module.initUno = function() {
    if (Module.uno === undefined) {
        Module.uno = init_unoembind_uno(Module);
    }
};

Module.unoObject = function(interfaces, obj) {
    Module.initUno();
    interfaces = ['com.sun.star.lang.XTypeProvider'].concat(interfaces);
    obj.impl_refcount = 0;
    obj.impl_types = new Module.uno_Sequence_type(interfaces.length, Module.uno_Sequence.FromSize);
    for (let i = 0; i !== interfaces.length; ++i) {
        obj.impl_types.set(i, Module.uno_Type.Interface(interfaces[i]));
    }
    obj.impl_implementationId = new Module.uno_Sequence_byte([]);
    obj.queryInterface = function(type) {
        for (const i in obj.impl_typemap) {
            if (i === type.toString()) {
                return new Module.uno_Any(
                    type,
                    Module['uno_Type_' + i.replace(/\./g, '$')].reference(
                        obj.impl_interfaces[obj.impl_typemap[i]]));
            }
        }
        return new Module.uno_Any(Module.uno_Type.Void(), undefined);
    };
    obj.acquire = function() { ++obj.impl_refcount; };
    obj.release = function() {
        if (--obj.impl_refcount === 0) {
            for (const i in obj.impl_interfaces) {
                i.delete();
            }
            obj.impl_types.delete();
            obj.impl_implementationId.delete();
        }
    };
    obj.getTypes = function() { return obj.impl_types; };
    obj.getImplementationId = function() { return obj.impl_implementationId; };
    obj.impl_interfaces = {};
    interfaces.forEach((i) => {
        obj.impl_interfaces[i] = Module['uno_Type_' + i.replace(/\./g, '$')].implement(obj);
    });
    obj.impl_typemap = {};
    const walk = function(td, impl) {
        const name = td.getName();
        if (!Object.hasOwn(obj.impl_typemap, name)) {
            if (td.getTypeClass() != Module.uno.com.sun.star.uno.TypeClass.INTERFACE) {
                throw new Error('not a UNO interface type: ' + name);
            }
            obj.impl_typemap[name] = impl;
            const bases = Module.uno.com.sun.star.reflection.XInterfaceTypeDescription2.query(td)
                  .getBaseTypes();
            for (let i = 0; i !== bases.size(); ++i) {
                walk(bases.get(i), impl)
            }
            bases.delete();
        }
    };
    const tdmAny = Module.getUnoComponentContext().getValueByName(
        '/singletons/com.sun.star.reflection.theTypeDescriptionManager');
    const tdm = Module.uno.com.sun.star.container.XHierarchicalNameAccess.query(tdmAny.get());
    interfaces.forEach((i) => {
        const td = tdm.getByHierarchicalName(i);
        walk(Module.uno.com.sun.star.reflection.XTypeDescription.query(td.get()), i);
        td.delete();
    })
    tdmAny.delete();
    return Module.uno.com.sun.star.uno.XInterface.reference(
        obj.impl_interfaces[obj.impl_typemap['com.sun.star.uno.XInterface']]);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
