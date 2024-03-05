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
    let test = new uno.org.libreoffice.embindtest.Test(Module.getUnoComponentContext());
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
        let a = new Module.Any(undefined, css.uno.TypeClass.VOID);
        console.assert(test.isAnyVoid(a));
        a.delete();
    }
    {
        let v = test.getAnyBoolean();
        console.log(v);
        console.assert(v.get() === true);
        console.assert(test.isAnyBoolean(v));
        v.delete();
        let a = new Module.Any(true, css.uno.TypeClass.BOOLEAN);
        console.assert(test.isAnyBoolean(a));
        a.delete();
    }
    {
        let v = test.getAnyByte();
        console.log(v);
        console.assert(v.get() === -12);
        console.assert(test.isAnyByte(v));
        v.delete();
        let a = new Module.Any(-12, css.uno.TypeClass.BYTE);
        console.assert(test.isAnyByte(a));
        a.delete();
    }
    {
        let v = test.getAnyShort();
        console.log(v);
        console.assert(v.get() === -1234);
        console.assert(test.isAnyShort(v));
        v.delete();
        let a = new Module.Any(-1234, css.uno.TypeClass.SHORT);
        console.assert(test.isAnyShort(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedShort();
        console.log(v);
        console.assert(v.get() === 54321);
        console.assert(test.isAnyUnsignedShort(v));
        v.delete();
        let a = new Module.Any(54321, css.uno.TypeClass.UNSIGNED_SHORT);
        console.assert(test.isAnyUnsignedShort(a));
        a.delete();
    }
    {
        let v = test.getAnyLong();
        console.log(v);
        console.assert(v.get() === -123456);
        console.assert(test.isAnyLong(v));
        v.delete();
        let a = new Module.Any(-123456, css.uno.TypeClass.LONG);
        console.assert(test.isAnyLong(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedLong();
        console.log(v);
        console.assert(v.get() === 3456789012);
        console.assert(test.isAnyUnsignedLong(v));
        v.delete();
        let a = new Module.Any(3456789012, css.uno.TypeClass.UNSIGNED_LONG);
        console.assert(test.isAnyUnsignedLong(a));
        a.delete();
    }
    {
        let v = test.getAnyHyper();
        console.log(v);
        console.assert(v.get() === -123456789n);
        console.assert(test.isAnyHyper(v));
        v.delete();
        let a = new Module.Any(-123456789n, css.uno.TypeClass.HYPER);
        console.assert(test.isAnyHyper(a));
        a.delete();
    }
    {
        let v = test.getAnyUnsignedHyper();
        console.log(v);
        console.assert(v.get() === 9876543210n);
        console.assert(test.isAnyUnsignedHyper(v));
        v.delete();
        let a = new Module.Any(9876543210n, css.uno.TypeClass.UNSIGNED_HYPER);
        console.assert(test.isAnyUnsignedHyper(a));
        a.delete();
    }
    {
        let v = test.getAnyFloat();
        console.log(v);
        console.assert(v.get() === -10.25);
        console.assert(test.isAnyFloat(v));
        v.delete();
        let a = new Module.Any(-10.25, css.uno.TypeClass.FLOAT);
        console.assert(test.isAnyFloat(a));
        a.delete();
    }
    {
        let v = test.getAnyDouble();
        console.log(v);
        console.assert(v.get() === 100.5);
        console.assert(test.isAnyDouble(v));
        v.delete();
        let a = new Module.Any(100.5, css.uno.TypeClass.DOUBLE);
        console.assert(test.isAnyDouble(a));
        a.delete();
    }
    {
        let v = test.getAnyChar();
        console.log(v);
        console.assert(v.get() === 'Ö');
        console.assert(test.isAnyChar(v));
        v.delete();
        let a = new Module.Any('Ö', css.uno.TypeClass.CHAR);
        console.assert(test.isAnyChar(a));
        a.delete();
    }
    {
        let v = test.getAnyString();
        console.log(v);
        console.assert(v.get() === 'hä');
        console.assert(test.isAnyString(v));
        v.delete();
        let a = new Module.Any('hä', css.uno.TypeClass.STRING);
        console.assert(test.isAnyString(a));
        a.delete();
    }
    {
        let v = test.getAnyType();
        console.log(v);
        console.assert(v.get().toString() === 'long');
        console.assert(test.isAnyType(v));
        v.delete();
        //TODO: let a = new Module.Any(TODO, css.uno.TypeClass.TYPE);
        //TODO: console.assert(test.isAnyType(a));
        //TODO: a.delete();
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
        //TODO: let a = new Module.Any(s, css.uno.TypeClass.SEQUENCE);
        //TODO: console.assert(test.isAnySequence(a));
        //TODO: a.delete();
        s.delete();
    }
    {
        let v = test.getAnyEnum();
        console.log(v);
        console.assert(v.get() === uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(test.isAnyEnum(v));
        v.delete();
        //TODO: let a = new Module.Any(
        //TODO:     uno.org.libreoffice.embindtest.Enum.E_2, css.uno.TypeClass.ENUM);
        //TODO: console.assert(test.isAnyEnum(a));
        //TODO: a.delete();
    }
    {
        let v = test.getAnyStruct();
        console.log(v);
        console.assert(v.get().m1 === -123456);
        console.assert(v.get().m2 === 100.5);
        console.assert(v.get().m3 === 'hä');
        console.assert(test.isAnyStruct(v));
        v.delete();
        //TODO: let a = new Module.Any(
        //TODO:     {m1: -123456, m2: 100.5, m3: 'hä'}, css.uno.TypeClass.STRUCT);
        //TODO: console.assert(test.isAnyStruct(a));
        //TODO: a.delete();
    }
    {
        let v = test.getAnyException();
        console.log(v);
        console.assert(v.get().Message === 'error');
        console.assert(v.get().Context === null);
        console.assert(v.get().m1 === -123456);
        console.assert(v.get().m2 === 100.5);
        console.assert(v.get().m3 === 'hä');
        console.assert(test.isAnyException(v));
        v.delete();
        //TODO: let a = new Module.Any(
        //TODO:     {Message: 'error', Context: null, m1: -123456, m2: 100.5, m3: 'hä'},
        //TODO:     css.uno.TypeClass.EXCEPTION);
        //TODO: console.assert(test.isAnyException(a));
        //TODO: a.delete();
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
        //TODO: let e2 = v.get(2);
        //TODO: console.assert(e2.size() === 3);
        //TODO: console.assert(e2.get(0) == uno.org.libreoffice.embindtest.Enum.E_2);
        //TODO: console.assert(e2.get(1) == uno.org.libreoffice.embindtest.Enum.E3);
        //TODO: console.assert(e2.get(2) == uno.org.libreoffice.embindtest.Enum.E_10);
        //TODO: e2.delete();
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
        console.assert(v.get(0) == uno.org.libreoffice.embindtest.Enum.E_2);
        console.assert(v.get(1) == uno.org.libreoffice.embindtest.Enum.E3);
        console.assert(v.get(2) == uno.org.libreoffice.embindtest.Enum.E_10);
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
});

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
