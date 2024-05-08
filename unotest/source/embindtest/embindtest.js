/* -*- Mode: JS; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

Module.addOnPostRun(function() {
    console.log('Running embindtest');
    let uno = init_unoembind_uno(Module);
    let css = uno.com.sun.star;
    let test = uno.org.libreoffice.embindtest.Test(Module.getUnoComponentContext());
    console.assert(typeof test === 'object');
    {
        let v = test.getBoolean();
        console.log(v);
        console.assert(v === 1); //TODO: true
        console.assert(test.isBoolean(v));
    }
    {
        let v = test.getByte();
        console.log(v);
        console.assert(v === -12);
        console.assert(test.isByte(v));
    }
    {
        let v = test.getShort();
        console.log(v);
        console.assert(v === -1234);
        console.assert(test.isShort(v));
    }
    {
        let v = test.getUnsignedShort();
        console.log(v);
        console.assert(v === 54321);
        console.assert(test.isUnsignedShort(v));
    }
    {
        let v = test.getLong();
        console.log(v);
        console.assert(v === -123456);
        console.assert(test.isLong(v));
    }
    {
        let v = test.getUnsignedLong();
        console.log(v);
        console.assert(v === 3456789012);
        console.assert(test.isUnsignedLong(v));
    }
    {
        let v = test.getHyper();
        console.log(v);
        console.assert(v === -123456789n);
        console.assert(test.isHyper(v));
    }
    {
        let v = test.getUnsignedHyper();
        console.log(v);
        console.assert(v === 9876543210n);
        console.assert(test.isUnsignedHyper(v));
    }
    {
        let v = test.getFloat();
        console.log(v);
        console.assert(v === -10.25);
        console.assert(test.isFloat(v));
    }
    {
        let v = test.getDouble();
        console.log(v);
        console.assert(v === 100.5);
        console.assert(test.isDouble(v));
    }
    {
        let v = test.getChar();
        console.log(v);
        console.assert(v === 'Ö');
        console.assert(test.isChar(v));
    }
    {
        let v = test.getString();
        console.log(v);
        console.assert(v === 'hä');
        console.assert(test.isString(v));
    }
    {
        let v = test.getType();
        console.log(v);
        console.assert(v.toString() === 'long');
        console.assert(test.isType(v));
        console.assert(test.isType(Module.uno_Type.Long()));
    }
    {
        let v = test.getEnum();
        console.log(v);
        console.assert(v === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(test.isEnum(v));
    }
    {
        let v = test.getStruct();
        console.log(v);
        console.assert(v.m1 === -123456);
        console.assert(v.m2 === 100.5);
        console.assert(v.m3 === 'hä');
        console.assert(test.isStruct(v));
    }
    {
        let v = test.getAnyVoid();
        console.log(v);
        console.assert(v.get() === undefined);
        console.assert(test.isAnyVoid(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Void(), undefined);
        console.assert(test.isAnyVoid(a));
        a.delete();
    }
    {
        let v = test.getAnyBoolean();
        console.log(v);
        console.assert(v.get() === true);
        console.assert(test.isAnyBoolean(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Boolean(), true);
        console.assert(test.isAnyBoolean(a));
        a.delete();
    }
    {
        let v = test.getAnyByte();
        console.log(v);
        console.assert(v.get() === -12);
        console.assert(test.isAnyByte(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Byte(), -12);
        console.assert(test.isAnyByte(a));
        a.delete();
    }
    {
        let v = test.getAnyShort();
        console.log(v);
        console.assert(v.get() === -1234);
        console.assert(test.isAnyShort(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Short(), -1234);
        console.assert(test.isAnyShort(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedShort();
        console.log(v);
        console.assert(v.get() === 54321);
        console.assert(test.isAnyUnsignedShort(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.UnsignedShort(), 54321);
        console.assert(test.isAnyUnsignedShort(a));
        a.delete();
    }
    {
        let v = test.getAnyLong();
        console.log(v);
        console.assert(v.get() === -123456);
        console.assert(test.isAnyLong(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Long(), -123456);
        console.assert(test.isAnyLong(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedLong();
        console.log(v);
        console.assert(v.get() === 3456789012);
        console.assert(test.isAnyUnsignedLong(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.UnsignedLong(), 3456789012);
        console.assert(test.isAnyUnsignedLong(a));
        a.delete();
    }
    {
        let v = test.getAnyHyper();
        console.log(v);
        console.assert(v.get() === -123456789n);
        console.assert(test.isAnyHyper(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Hyper(), -123456789n);
        console.assert(test.isAnyHyper(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedHyper();
        console.log(v);
        console.assert(v.get() === 9876543210n);
        console.assert(test.isAnyUnsignedHyper(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.UnsignedHyper(), 9876543210n);
        console.assert(test.isAnyUnsignedHyper(a));
        a.delete();
    }
    {
        let v = test.getAnyFloat();
        console.log(v);
        console.assert(v.get() === -10.25);
        console.assert(test.isAnyFloat(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Float(), -10.25);
        console.assert(test.isAnyFloat(a));
        a.delete();
    }
    {
        let v = test.getAnyDouble();
        console.log(v);
        console.assert(v.get() === 100.5);
        console.assert(test.isAnyDouble(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Double(), 100.5);
        console.assert(test.isAnyDouble(a));
        a.delete();
    }
    {
        let v = test.getAnyChar();
        console.log(v);
        console.assert(v.get() === 'Ö');
        console.assert(test.isAnyChar(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Char(), 'Ö');
        console.assert(test.isAnyChar(a));
        a.delete();
    }
    {
        let v = test.getAnyString();
        console.log(v);
        console.assert(v.get() === 'hä');
        console.assert(test.isAnyString(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.String(), 'hä');
        console.assert(test.isAnyString(a));
        a.delete();
    }
    {
        let v = test.getAnyType();
        console.log(v);
        console.assert(v.get().toString() === 'long');
        console.assert(test.isAnyType(v));
        v.delete();
        let a = new Module.uno_Any(Module.uno_Type.Type(), Module.uno_Type.Long());
        console.assert(test.isAnyType(a));
        a.delete();
    }
    {
        let v = test.getAnySequence();
        console.log(v);
        let x = v.get();
        console.assert(x.size() === 3);
        console.assert(x.get(0) === 'foo');
        console.assert(x.get(1) === 'barr');
        console.assert(x.get(2) === 'bazzz');
        x.delete();
        console.assert(test.isAnySequence(v));
        v.delete();
        let s = new Module.uno_Sequence_string(["foo", "barr", "bazzz"]);
        let a = new Module.uno_Any(Module.uno_Type.Sequence(Module.uno_Type.String()), s);
        console.assert(test.isAnySequence(a));
        a.delete();
        s.delete();
    }
    {
        let v = test.getAnyEnum();
        console.log(v);
        console.assert(v.get() === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(test.isAnyEnum(v));
        v.delete();
        let a = new Module.uno_Any(
            Module.uno_Type.Enum('org.libreoffice.embindtest.Enum'),
            uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(test.isAnyEnum(a));
        a.delete();
    }
    {
        let v = test.getAnyStruct();
        console.log(v);
        console.assert(v.get().m1 === -123456);
        console.assert(v.get().m2 === 100.5);
        console.assert(v.get().m3 === 'hä');
        console.assert(test.isAnyStruct(v));
        v.delete();
        let a = new Module.uno_Any(
            Module.uno_Type.Struct('org.libreoffice.embindtest.Struct'),
            {m1: -123456, m2: 100.5, m3: 'hä'});
        console.assert(test.isAnyStruct(a));
        a.delete();
    }
    {
        let v = test.getAnyException();
        console.log(v);
        console.assert(v.get().Message.startsWith('error'));
        console.assert(v.get().Context === null);
        console.assert(v.get().m1 === -123456);
        console.assert(v.get().m2 === 100.5);
        console.assert(v.get().m3 === 'hä');
        console.assert(test.isAnyException(v));
        v.delete();
        let a = new Module.uno_Any(
            Module.uno_Type.Exception('org.libreoffice.embindtest.Exception'),
            {Message: 'error', Context: null, m1: -123456, m2: 100.5, m3: 'hä'});
        console.assert(test.isAnyException(a));
        a.delete();
    }
    {
        let v = test.getAnyInterface();
        console.log(v);
        console.assert(Module.sameUnoObject(v.get(), test));
        console.assert(test.isAnyInterface(v));
        v.delete();
        let a = new Module.uno_Any(
            Module.uno_Type.Interface('org.libreoffice.embindtest.XTest'), test);
        console.assert(test.isAnyInterface(a));
        a.delete();
    }
    {
        let v = test.getSequenceBoolean();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 1); //TODO: true
        console.assert(v.get(1) === 1); //TODO: true
        console.assert(v.get(2) === 0); //TODO: false
        console.assert(test.isSequenceBoolean(v));
        v.delete();
    }
    {
        let v = test.getSequenceByte();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -12);
        console.assert(v.get(1) === 1);
        console.assert(v.get(2) === 12);
        console.assert(test.isSequenceByte(v));
        v.delete();
    }
    {
        let v = test.getSequenceShort();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -1234);
        console.assert(v.get(1) === 1);
        console.assert(v.get(2) === 1234);
        console.assert(test.isSequenceShort(v));
        v.delete();
    }
    {
        let v = test.getSequenceUnsignedShort();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 1);
        console.assert(v.get(1) === 10);
        console.assert(v.get(2) === 54321);
        console.assert(test.isSequenceUnsignedShort(v));
        v.delete();
    }
    {
        let v = test.getSequenceLong();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -123456);
        console.assert(v.get(1) === 1);
        console.assert(v.get(2) === 123456);
        console.assert(test.isSequenceLong(v));
        v.delete();
    }
    {
        let v = test.getSequenceUnsignedLong();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 1);
        console.assert(v.get(1) === 10);
        console.assert(v.get(2) === 3456789012);
        console.assert(test.isSequenceUnsignedLong(v));
        v.delete();
    }
    {
        let v = test.getSequenceHyper();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -123456789n);
        console.assert(v.get(1) === 1n);
        console.assert(v.get(2) === 123456789n);
        console.assert(test.isSequenceHyper(v));
        v.delete();
    }
    {
        let v = test.getSequenceUnsignedHyper();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 1n);
        console.assert(v.get(1) === 10n);
        console.assert(v.get(2) === 9876543210n);
        console.assert(test.isSequenceUnsignedHyper(v));
        v.delete();
    }
    {
        let v = test.getSequenceFloat();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -10.25);
        console.assert(v.get(1) === 1.5);
        console.assert(v.get(2) === 10.75);
        console.assert(test.isSequenceFloat(v));
        v.delete();
    }
    {
        let v = test.getSequenceDouble();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === -100.5);
        console.assert(v.get(1) === 1.25);
        console.assert(v.get(2) === 100.75);
        console.assert(test.isSequenceDouble(v));
        v.delete();
    }
    {
        let v = test.getSequenceChar();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 'a');
        console.assert(v.get(1) === 'B');
        console.assert(v.get(2) === 'Ö');
        console.assert(test.isSequenceChar(v));
        v.delete();
    }
    {
        let v = test.getSequenceString();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === 'foo');
        console.assert(v.get(1) === 'barr');
        console.assert(v.get(2) === 'bazzz');
        console.assert(test.isSequenceString(v));
        v.delete();
    }
    {
        let v = test.getSequenceType();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0).toString() === 'long');
        console.assert(v.get(1).toString() === 'void');
        console.assert(v.get(2).toString() === '[]org.libreoffice.embindtest.Enum');
        console.assert(test.isSequenceType(v));
        v.delete();
        let s = new Module.uno_Sequence_type([
            Module.uno_Type.Long(), Module.uno_Type.Void(),
            Module.uno_Type.Sequence(Module.uno_Type.Enum('org.libreoffice.embindtest.Enum'))]);
        console.assert(test.isSequenceType(s));
        s.delete();
    }
    {
        let v = test.getSequenceAny();
        console.log(v);
        console.assert(v.size() === 3);
        let e0 = v.get(0);
        console.assert(e0.get() === -123456);
        e0.delete();
        let e1 = v.get(1);
        console.assert(e1.get() === undefined);
        e1.delete();
        let e2 = v.get(2);
        let s = e2.get();
        console.assert(s.size() === 3);
        console.assert(s.get(0) === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(s.get(1) === uno.org.libreoffice.embindtest.Enum.E3);
        console.assert(s.get(2) === uno.org.libreoffice.embindtest.Enum.E_10);
        s.delete();
        e2.delete();
        console.assert(test.isSequenceAny(v));
        v.delete();
    }
    {
        let v = test.getSequenceSequenceString();
        console.log(v);
        console.assert(v.size() === 3);
        let e0 = v.get(0);
        console.assert(e0.size() === 0);
        e0.delete();
        let e1 = v.get(1);
        console.assert(e1.size() === 2);
        console.assert(e1.get(0) === 'foo');
        console.assert(e1.get(1) === 'barr');
        e1.delete();
        let e2 = v.get(2);
        console.assert(e2.size() === 1);
        console.assert(e2.get(0) === 'baz');
        e2.delete();
        console.assert(test.isSequenceSequenceString(v));
        v.delete();
    }
    {
        let v = test.getSequenceEnum();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0) === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(v.get(1) === uno.org.libreoffice.embindtest.Enum.E3);
        console.assert(v.get(2) === uno.org.libreoffice.embindtest.Enum.E_10);
        console.assert(test.isSequenceEnum(v));
        v.delete();
    }
    {
        let v = test.getSequenceStruct();
        console.log(v);
        console.assert(v.size() === 3);
        console.assert(v.get(0).m1 === -123456);
        console.assert(v.get(0).m2 === -100.5);
        console.assert(v.get(0).m3 === 'foo');
        console.assert(v.get(1).m1 === 1);
        console.assert(v.get(1).m2 === 1.25);
        console.assert(v.get(1).m3 === 'barr');
        console.assert(v.get(2).m1 === 123456);
        console.assert(v.get(2).m2 === 100.75);
        console.assert(v.get(2).m3 === 'bazzz');
        console.assert(test.isSequenceStruct(v));
        v.delete();
    }
    {
        let v = test.getNull();
        console.log(v);
        console.assert(v === null);
        console.assert(test.isNull(v));
    }
    {
        let v = css.task.XJob.query(test);
        console.log(v);
        console.assert(v === null);
    }
    {
        const v1 = new Module.uno_InOutParam_boolean;
        const v2 = new Module.uno_InOutParam_byte;
        const v3 = new Module.uno_InOutParam_short;
        const v4 = new Module.uno_InOutParam_unsigned_short;
        const v5 = new Module.uno_InOutParam_long;
        const v6 = new Module.uno_InOutParam_unsigned_long;
        const v7 = new Module.uno_InOutParam_hyper;
        const v8 = new Module.uno_InOutParam_unsigned_hyper;
        const v9 = new Module.uno_InOutParam_float;
        const v10 = new Module.uno_InOutParam_double;
        const v11 = new Module.uno_InOutParam_char;
        const v12 = new Module.uno_InOutParam_string;
        const v13 = new Module.uno_InOutParam_type;
        const v14 = new Module.uno_InOutParam_any;
        const v15 = new Module.uno_InOutParam_sequence_string;
        const v16 = new Module.uno_InOutParam_org$libreoffice$embindtest$Enum;
        const v17 = new Module.uno_InOutParam_org$libreoffice$embindtest$Struct;
        const v18 = new Module.uno_InOutParam_org$libreoffice$embindtest$XTest;
        test.getOut(
            v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
        console.log(v1.val);
        console.log(v2.val);
        console.log(v3.val);
        console.log(v4.val);
        console.log(v5.val);
        console.log(v6.val);
        console.log(v7.val);
        console.log(v8.val);
        console.log(v9.val);
        console.log(v10.val);
        console.log(v11.val);
        console.log(v12.val);
        console.log(v13.val);
        console.log(v14.val);
        console.log(v15.val);
        console.log(v16.val);
        console.log(v17.val);
        console.log(v18.val);
        console.assert(v1.val === 1); //TODO: true
        console.assert(v2.val === -12);
        console.assert(v3.val === -1234);
        console.assert(v4.val === 54321);
        console.assert(v5.val === -123456);
        console.assert(v6.val === 3456789012);
        console.assert(v7.val === -123456789n);
        console.assert(v8.val === 9876543210n);
        console.assert(v9.val === -10.25);
        console.assert(v10.val === 100.5);
        console.assert(v11.val === 'Ö');
        console.assert(v12.val === 'hä');
        console.assert(v13.val.toString() === 'long');
        console.assert(v14.val.get() === -123456)
        console.assert(v15.val.size() === 3);
        console.assert(v15.val.get(0) === 'foo');
        console.assert(v15.val.get(1) === 'barr');
        console.assert(v15.val.get(2) === 'bazzz');
        console.assert(v16.val === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(v17.val.m1 === -123456);
        console.assert(v17.val.m2 === 100.5);
        console.assert(v17.val.m3 === 'hä');
        console.assert(Module.sameUnoObject(v18.val, test));
        v1.delete();
        v2.delete();
        v3.delete();
        v4.delete();
        v5.delete();
        v6.delete();
        v7.delete();
        v8.delete();
        v9.delete();
        v10.delete();
        v11.delete();
        v12.delete();
        v13.delete();
        v14.val.delete();
        v14.delete();
        v15.val.delete();
        v15.delete();
        v16.delete();
        v17.delete();
        v18.delete();
    }
    console.assert(uno.org.libreoffice.embindtest.Constants.Boolean === true);
    console.assert(test.isBoolean(uno.org.libreoffice.embindtest.Constants.Boolean));
    console.assert(uno.org.libreoffice.embindtest.Constants.Byte === -12);
    console.assert(test.isByte(uno.org.libreoffice.embindtest.Constants.Byte));
    console.assert(uno.org.libreoffice.embindtest.Constants.Short === -1234);
    console.assert(test.isShort(uno.org.libreoffice.embindtest.Constants.Short));
    console.assert(uno.org.libreoffice.embindtest.Constants.UnsignedShort === 54321);
    console.assert(test.isUnsignedShort(uno.org.libreoffice.embindtest.Constants.UnsignedShort));
    console.assert(uno.org.libreoffice.embindtest.Constants.Long === -123456);
    console.assert(test.isLong(uno.org.libreoffice.embindtest.Constants.Long));
    console.assert(uno.org.libreoffice.embindtest.Constants.UnsignedLong === 3456789012);
    console.assert(test.isUnsignedLong(uno.org.libreoffice.embindtest.Constants.UnsignedLong));
    console.assert(uno.org.libreoffice.embindtest.Constants.Hyper === -123456789n);
    console.assert(test.isHyper(uno.org.libreoffice.embindtest.Constants.Hyper));
    console.assert(uno.org.libreoffice.embindtest.Constants.UnsignedHyper === 9876543210n);
    console.assert(test.isUnsignedHyper(uno.org.libreoffice.embindtest.Constants.UnsignedHyper));
    console.assert(uno.org.libreoffice.embindtest.Constants.Float === -10.25);
    console.assert(test.isFloat(uno.org.libreoffice.embindtest.Constants.Float));
    console.assert(uno.org.libreoffice.embindtest.Constants.Double === 100.5);
    console.assert(test.isDouble(uno.org.libreoffice.embindtest.Constants.Double));
    try {
        test.throwRuntimeException();
    } catch (e) {
        incrementExceptionRefcount(e);
            //TODO, needed when building with JS-based -fexceptions, see
            // <https://github.com/emscripten-core/emscripten/issues/17115> "[EH] Fix inconsistency
            // of refcounting in Emscripten EH vs. Wasm EH"
        console.assert(e.name === 'com::sun::star::uno::RuntimeException');
        console.assert(e.message === undefined); //TODO
        //TODO: console.assert(e.Message.startsWith('test'));
        decrementExceptionRefcount(e);
    }
    const obj = {
        implRefcount: 0,
        implTypes: new Module.uno_Sequence_type([
            Module.uno_Type.Interface('com.sun.star.lang.XTypeProvider'),
            Module.uno_Type.Interface('com.sun.star.task.XJob'),
            Module.uno_Type.Interface('com.sun.star.task.XJobExecutor')]),
        implImplementationId: new Module.uno_Sequence_byte([]),
        queryInterface(type) {
            if (type == 'com.sun.star.uno.XInterface') {
                return new Module.uno_Any(
                    type, css.uno.XInterface.reference(this.implXTypeProvider));
            } else if (type == 'com.sun.star.lang.XTypeProvider') {
                return new Module.uno_Any(
                    type, css.lang.XTypeProvider.reference(this.implXTypeProvider));
            } else if (type == 'com.sun.star.task.XJob') {
                return new Module.uno_Any(type, css.task.XJob.reference(this.implXJob));
            } else if (type == 'com.sun.star.task.XJobExecutor') {
                return new Module.uno_Any(
                    type, css.task.XJobExecutor.reference(this.implXJobExecutor));
            } else {
                return new Module.uno_Any(Module.uno_Type.Void(), undefined);
            }
        },
        acquire() { ++this.implRefcount; },
        release() {
            if (--this.implRefcount === 0) {
                this.implXTypeProvider.delete();
                this.implXJob.delete();
                this.implXJobExecutor.delete();
                this.implTypes.delete();
                this.implImplementationId.delete();
            }
        },
        getTypes() { return this.implTypes; },
        getImplementationId() { return this.implImplementationId; },
        execute(args) {
            if (args.size() !== 1 || args.get(0).Name !== 'name') {
                Module.throwUnoException(
                    Module.uno_Type.Exception('com.sun.star.lang.IllegalArgumentException'),
                    {Message: 'bad args', Context: null, ArgumentPosition: 0});
            }
            console.log('Hello ' + args.get(0).Value.get());
            return new Module.uno_Any(Module.uno_Type.Void(), undefined);
        },
        trigger(event) { console.log('Ola ' + event); }
    };
    obj.implXTypeProvider = css.lang.XTypeProvider.implement(obj);
    obj.implXJob = css.task.XJob.implement(obj);
    obj.implXJobExecutor = css.task.XJobExecutor.implement(obj);
    obj.acquire();
    test.passJob(css.task.XJob.reference(obj.implXJob));
    test.passJobExecutor(css.task.XJobExecutor.reference(obj.implXJobExecutor));
    test.passInterface(css.uno.XInterface.reference(obj.implXTypeProvider));
    obj.release();

    const args = new Module.uno_Sequence_any(
        [new Module.uno_Any(Module.uno_Type.Interface('com.sun.star.uno.XInterface'), test)]);
    const invoke = css.script.XInvocation2.query(css.script.Invocation.create(
        Module.getUnoComponentContext()).createInstanceWithArguments(args));
    args.get(0).delete();
    args.delete();
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getBoolean', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isBoolean', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getByte', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isByte', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getShort', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isShort', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getUnsignedShort', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isUnsignedShort', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getLong', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isLong', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getUnsignedLong', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isUnsignedLong', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getHyper', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isHyper', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getUnsignedHyper', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isUnsignedHyper', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getFloat', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isFloat', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getDouble', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isDouble', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getChar', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isChar', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getString', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isString', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getType', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isType', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getEnum', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isEnum', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getStruct', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isStruct', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getStructLong', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isStructLong', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getStructString', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isStructString', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getAnyLong', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isAnyLong', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getSequenceLong', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isSequenceLong', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params1 = new Module.uno_Sequence_any(0, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret1 = invoke.invoke('getNull', params1, outparamindex, outparam);
        console.log(ret1.get());
        const params2 = new Module.uno_Sequence_any([ret1]);
        const ret2 = invoke.invoke('isNull', params2, outparamindex, outparam);
        console.log(ret2.get());
        console.assert(ret2.get());
        ret1.delete();
        params1.delete();
        ret2.delete();
        params2.delete();
        outparamindex.delete();
        outparam.delete();
    }
    {
        const params = new Module.uno_Sequence_any(18, Module.uno_Sequence.FromSize);
        const outparamindex = new Module.uno_InOutParam_sequence_short;
        const outparam = new Module.uno_InOutParam_sequence_any;
        const ret = invoke.invoke('getOut', params, outparamindex, outparam);
        console.assert(ret.get() === undefined);
        ret.delete();
        params.delete();
        console.assert(outparamindex.val.size() == 18);
        outparamindex.val.delete();
        outparamindex.delete();
        console.assert(outparam.val.size() == 18);
        console.assert(test.isBoolean(outparam.val.get(0).get()));
        outparam.val.get(0).delete();
        console.assert(test.isByte(outparam.val.get(1).get()));
        outparam.val.get(1).delete();
        console.assert(test.isShort(outparam.val.get(2).get()));
        outparam.val.get(2).delete();
        console.assert(test.isUnsignedShort(outparam.val.get(3).get()));
        outparam.val.get(3).delete();
        console.assert(test.isLong(outparam.val.get(4).get()));
        outparam.val.get(4).delete();
        console.assert(test.isUnsignedLong(outparam.val.get(5).get()));
        outparam.val.get(5).delete();
        console.assert(test.isHyper(outparam.val.get(6).get()));
        outparam.val.get(6).delete();
        console.assert(test.isUnsignedHyper(outparam.val.get(7).get()));
        outparam.val.get(7).delete();
        console.assert(test.isFloat(outparam.val.get(8).get()));
        outparam.val.get(8).delete();
        console.assert(test.isDouble(outparam.val.get(9).get()));
        outparam.val.get(9).delete();
        console.assert(test.isChar(outparam.val.get(10).get()));
        outparam.val.get(10).delete();
        console.assert(test.isString(outparam.val.get(11).get()));
        outparam.val.get(11).delete();
        console.assert(test.isType(outparam.val.get(12).get()));
        outparam.val.get(12).delete();
        console.assert(test.isAnyLong(outparam.val.get(13)));
        outparam.val.get(13).delete();
        console.assert(test.isSequenceString(outparam.val.get(14).get()));
        outparam.val.get(14).get().delete();
        outparam.val.get(14).delete();
        console.assert(test.isEnum(outparam.val.get(15).get()));
        outparam.val.get(15).delete();
        console.assert(test.isStruct(outparam.val.get(16).get()));
        outparam.val.get(16).delete();
        console.assert(Module.sameUnoObject(outparam.val.get(17).get(), test));
        outparam.val.get(17).delete();
        outparam.val.delete();
        outparam.delete();
    }
});

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
