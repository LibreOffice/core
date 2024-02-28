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
        let v = test.getStruct();
        console.log(v.m1 + ', ' + v.m2 + ', ' + v.m3);
        console.assert(v.m1 === -123456);
        console.assert(v.m2 === 100.5);
        console.assert(v.m3 === 'hä');
        console.assert(test.isStruct(v));
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
