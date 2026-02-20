/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <jsuno/jsuno.hxx>
#include <rtl/ustring.hxx>

namespace
{
class TestUno : public CppUnit::TestFixture
{
public:
    void test()
    {
        // The string literal needs to be cut into multiple parts to avoid MSVC "error C2026: string
        // too big, trailing characters truncated":
        jsuno::execute(uR"(
console.assert(uno.type.void.toString() == 'void');
console.assert(uno.type.boolean.toString() == 'boolean');
console.assert(uno.type.byte.toString() == 'byte');
console.assert(uno.type.short.toString() == 'short');
console.assert(uno.type.unsigned_short.toString() == 'unsigned short');
console.assert(uno.type.long.toString() == 'long');
console.assert(uno.type.unsigned_long.toString() == 'unsigned long');
console.assert(uno.type.hyper.toString() == 'hyper');
console.assert(uno.type.unsigned_hyper.toString() == 'unsigned hyper');
console.assert(uno.type.float.toString() == 'float');
console.assert(uno.type.double.toString() == 'double');
console.assert(uno.type.char.toString() == 'char');
console.assert(uno.type.string.toString() == 'string');
console.assert(uno.type.type.toString() == 'type');
console.assert(uno.type.any.toString() == 'any');
console.assert(uno.type.sequence(uno.type.sequence(uno.type.string)).toString() == '[][]string');
console.assert(
    uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)).toString()
    == '[]com.sun.star.testuno.Enum');
console.assert(
    uno.type.enum(uno.idl.com.sun.star.testuno.Enum).toString() == 'com.sun.star.testuno.Enum');
console.assert(
    uno.type.struct(uno.idl.com.sun.star.testuno.Struct).toString()
    == 'com.sun.star.testuno.Struct');
console.assert(
    uno.type.struct(
        uno.idl.com.sun.star.testuno.Template,
        [uno.type.any, uno.type.struct(uno.idl.com.sun.star.testuno.StructString)]).toString()
    == 'com.sun.star.testuno.Template<any,com.sun.star.testuno.StructString>');
console.assert(
    uno.type.exception(uno.idl.com.sun.star.testuno.Exception).toString()
    == 'com.sun.star.testuno.Exception');
console.assert(
    uno.type.interface(uno.idl.com.sun.star.testuno.XTest).toString()
    == 'com.sun.star.testuno.XTest');
console.assert(uno.idl.com.sun.star.testuno.Constants.Boolean === true);
console.assert(uno.idl.com.sun.star.testuno.Constants.Byte === -12);
console.assert(uno.idl.com.sun.star.testuno.Constants.Short === -1234);
console.assert(uno.idl.com.sun.star.testuno.Constants.UnsignedShort === 54321);
console.assert(uno.idl.com.sun.star.testuno.Constants.Long === -123456);
console.assert(uno.idl.com.sun.star.testuno.Constants.UnsignedLong === 3456789012);
console.assert(uno.idl.com.sun.star.testuno.Constants.Hyper === -123456789n);
console.assert(uno.idl.com.sun.star.testuno.Constants.UnsignedHyper === 9876543210n);
console.assert(uno.idl.com.sun.star.testuno.Constants.Float === -10.25);
console.assert(uno.idl.com.sun.star.testuno.Constants.Double === 100.5);
const test = uno.idl.com.sun.star.testuno.Test.create(uno.componentContext);
{
    const v = test.getBoolean();
    console.log(v);
    console.assert(v === true);
    console.assert(test.isBoolean(true));
}
{
    const v = test.getByte();
    console.log(v);
    console.assert(v === -12);
    console.assert(test.isByte(-12));
}
{
    const v = test.getShort();
    console.log(v);
    console.assert(v === -1234);
    console.assert(test.isShort(-1234));
}
{
    const v = test.getUnsignedShort();
    console.log(v);
    console.assert(v === 54321);
    console.assert(test.isUnsignedShort(54321));
}
{
    const v = test.getLong();
    console.log(v);
    console.assert(v === -123456);
    console.assert(test.isLong(-123456));
}
{
    const v = test.getUnsignedLong();
    console.log(v);
    console.assert(v === 3456789012);
    console.assert(test.isUnsignedLong(3456789012));
}
{
    const v = test.getHyper();
    console.log(v);
    console.assert(v === -123456789n);
    console.assert(test.isHyper(-123456789n));
}
{
    const v = test.getUnsignedHyper();
    console.log(v);
    console.assert(v === 9876543210n);
    console.assert(test.isUnsignedHyper(9876543210n));
}
{
    const v = test.getFloat();
    console.log(v);
    console.assert(v === -10.25);
    console.assert(test.isFloat(-10.25));
}
{
    const v = test.getDouble();
    console.log(v);
    console.assert(v === 100.5);
    console.assert(test.isDouble(100.5));
}
{
    const v = test.getChar();
    console.log(v);
    console.assert(v === 'Ö');
    console.assert(test.isChar('Ö'));
}
{
    const v = test.getString();
    console.log(v);
    console.assert(v === 'hä');
    console.assert(test.isString('hä'));
}
{
    const v = test.getType();
    console.log(v);
    console.assert(v.toString() === 'long');
    console.assert(test.isType(v));
    console.assert(test.isType(uno.type.long));
}
{
    const v = test.getEnum();
    console.log(v);
    console.assert(v === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(test.isEnum(uno.idl.com.sun.star.testuno.Enum.E_2));
}
{
    const v = test.getStruct();
    console.log(v);
    console.assert(v.m1 === true);
    console.assert(v.m2 === -12);
    console.assert(v.m3 === -1234);
    console.assert(v.m4 === 54321);
    console.assert(v.m5 === -123456);
    console.assert(v.m6 === 3456789012);
    console.assert(v.m7 === -123456789n);
    console.assert(v.m8 === 9876543210n);
    console.assert(v.m9 === -10.25);
    console.assert(v.m10 === 100.5);
    console.assert(v.m11 === 'Ö');
    console.assert(v.m12 === 'hä');
    console.assert(v.m13.toString() === 'long');
    console.assert(v.m14 === -123456);
    console.assert(v.m15.length === 3);
    console.assert(v.m15[0] === 'foo');
    console.assert(v.m15[1] === 'barr');
    console.assert(v.m15[2] === 'bazzz');
    console.assert(v.m16 === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v.m17.m === -123456);
    console.assert(v.m18.m1.m === 'foo');
    console.assert(v.m18.m2 === -123456);
    console.assert(v.m18.m3 === -123456);
    console.assert(v.m18.m4.m === 'barr');
    console.assert(uno.sameUnoObject(v.m19, test));
    console.assert(test.isStruct(
        new uno.idl.com.sun.star.testuno.Struct({
            m1: true, m2: -12, m3: -1234, m4: 54321, m5: -123456, m6: 3456789012,
            m7: -123456789n, m8: 9876543210n, m9: -10.25, m10: 100.5, m11: 'Ö', m12: 'hä',
            m13: uno.type.long, m14: -123456, m15: ['foo', 'barr', 'bazzz'],
            m16: uno.idl.com.sun.star.testuno.Enum.E_2,
            m17: new uno.idl.com.sun.star.testuno.StructLong({m: -123456}),
            m18: new uno.idl.com.sun.star.testuno.Template(
                [uno.type.any,
                 uno.type.struct(
                     uno.idl.com.sun.star.testuno.StructString)],
                {m1: new uno.idl.com.sun.star.testuno.StructString({m: 'foo'}),
                 m2: -123456, m3: -123456,
                 m4: new uno.idl.com.sun.star.testuno.StructString({m: 'barr'})}),
            m19: test})));
    console.assert(test.isStruct({
        m1: true, m2: -12, m3: -1234, m4: 54321, m5: -123456, m6: 3456789012,
        m7: -123456789n, m8: 9876543210n, m9: -10.25, m10: 100.5, m11: 'Ö', m12: 'hä',
        m13: uno.type.long, m14: -123456, m15: ['foo', 'barr', 'bazzz'],
        m16: uno.idl.com.sun.star.testuno.Enum.E_2, m17: {m: -123456},
        m18: {m1: {m: 'foo'}, m2: -123456, m3: -123456, m4: {m: 'barr'}}, m19: test}));
    const def = new uno.idl.com.sun.star.testuno.Struct();
    console.assert(def.m1 === false);
    console.assert(def.m2 === 0);
    console.assert(def.m3 === 0);
    console.assert(def.m4 === 0);
    console.assert(def.m5 === 0);
    console.assert(def.m6 === 0);
    console.assert(def.m7 === 0n);
    console.assert(def.m8 === 0n);
    console.assert(def.m9 === 0);
    console.assert(def.m10 === 0);
    console.assert(def.m11 === '\0');
    console.assert(def.m12 === '');
    console.assert(def.m13.toString() === 'void');
    console.assert(def.m14 === undefined);
    console.assert(def.m15.length === 0);
    console.assert(def.m16 === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(def.m17.m === 0);
    console.assert(def.m18.m1.m === '');
    console.assert(def.m18.m2 === 0);
    console.assert(def.m18.m3 === undefined);
    console.assert(def.m18.m4.m === '');
    console.assert(def.m19 === null);
}
)"
                       uR"(
{
    const v = test.getTemplate();
    console.log(v);
    console.assert(v.m1.m === 'foo');
    console.assert(v.m2 === -123456);
    console.assert(v.m3 === -123456);
    console.assert(v.m4.m === 'barr');
    console.assert(test.isTemplate(
        new uno.idl.com.sun.star.testuno.Template(
            [uno.type.any, uno.type.struct(uno.idl.com.sun.star.testuno.StructString)],
            {m1: new uno.idl.com.sun.star.testuno.StructString({m: 'foo'}),
             m2: -123456, m3: -123456,
             m4: new uno.idl.com.sun.star.testuno.StructString({m: 'barr'})})));
    console.assert(test.isTemplate(
        {m1: {m: 'foo'}, m2: -123456, m3: -123456, m4: {m: 'barr'}}));
    const def = new uno.idl.com.sun.star.testuno.Template(
        [uno.type.any, uno.type.struct(uno.idl.com.sun.star.testuno.StructString)]);
    console.assert(def.m1.m === '');
    console.assert(def.m2 === 0);
    console.assert(def.m3 === undefined);
    console.assert(def.m4.m === '');
}
{
    const v = test.getAnyVoid();
    console.log(v);
    console.assert(v === undefined);
    console.assert(test.isAnyVoid(undefined));
    console.assert(test.isAnyVoid(new uno.Any(uno.type.void)));
}
{
    const v = test.getAnyBoolean();
    console.log(v);
    console.assert(v === true);
    console.assert(test.isAnyBoolean(true));
    console.assert(test.isAnyBoolean(new uno.Any(uno.type.boolean, true)));
}
{
    const v = test.getAnyByte();
    console.log(v);
    console.assert(v === -12);
    console.assert(test.isAnyByte(new uno.Any(uno.type.byte, -12)));
}
{
    const v = test.getAnyShort();
    console.log(v);
    console.assert(v === -1234);
    console.assert(test.isAnyShort(new uno.Any(uno.type.short, -1234)));
}
{
    const v = test.getAnyUnsignedShort();
    console.log(v);
    console.assert(v === 54321);
    console.assert(test.isAnyUnsignedShort(new uno.Any(uno.type.unsigned_short, 54321)));
}
{
    const v = test.getAnyLong();
    console.log(v);
    console.assert(v === -123456);
    console.assert(test.isAnyLong(-123456));
    console.assert(test.isAnyLong(new uno.Any(uno.type.long, -123456)));
}
{
    const v = test.getAnyUnsignedLong();
    console.log(v);
    console.assert(v === 3456789012);
    console.assert(test.isAnyUnsignedLong(3456789012));
    console.assert(test.isAnyUnsignedLong(new uno.Any(uno.type.unsigned_long, 3456789012)));
}
{
    const v = test.getAnyHyper();
    console.log(v);
    console.assert(v === -123456789n);
    console.assert(test.isAnyHyper(-123456789n));
    console.assert(test.isAnyHyper(new uno.Any(uno.type.hyper, -123456789n)));
}
{
    const v = test.getAnyUnsignedHyper();
    console.log(v);
    console.assert(v === 9876543210n);
    console.assert(test.isAnyUnsignedHyper(new uno.Any(uno.type.unsigned_hyper, 9876543210n)));
}
{
    const v = test.getAnyFloat();
    console.log(v);
    console.assert(v === -10.25);
    console.assert(test.isAnyFloat(new uno.Any(uno.type.float, -10.25)));
}
{
    const v = test.getAnyDouble();
    console.log(v);
    console.assert(v === 100.5);
    console.assert(test.isAnyDouble(100.5));
    console.assert(test.isAnyDouble(new uno.Any(uno.type.double, 100.5)));
}
{
    const v = test.getAnyChar();
    console.log(v);
    console.assert(v === 'Ö');
    console.assert(test.isAnyChar(new uno.Any(uno.type.char, 'Ö')));
}
{
    const v = test.getAnyString();
    console.log(v);
    console.assert(v === 'hä');
    console.assert(test.isAnyString('hä'));
    console.assert(test.isAnyString(new uno.Any(uno.type.string, 'hä')));
}
{
    const v = test.getAnyType();
    console.log(v);
    console.assert(v.toString() === 'long');
    console.assert(test.isAnyType(uno.type.long));
    console.assert(test.isAnyType(new uno.Any(uno.type.type, uno.type.long)));
}
{
    const v = test.getAnySequence();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 'foo');
    console.assert(v[1] === 'barr');
    console.assert(v[2] === 'bazzz');
    console.assert(test.isAnySequence(
        new uno.Any(uno.type.sequence(uno.type.string), ['foo', 'barr', 'bazzz'])));
}
{
    const v = test.getAnyEnum();
    console.log(v);
    console.assert(v === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(test.isAnyEnum(uno.idl.com.sun.star.testuno.Enum.E_2));
    console.assert(test.isAnyEnum(
        new uno.Any(
            uno.type.enum(uno.idl.com.sun.star.testuno.Enum),
            uno.idl.com.sun.star.testuno.Enum.E_2)));
}
{
    const v = test.getAnyStruct();
    console.log(v);
    console.assert(v.m1 === true);
    console.assert(v.m2 === -12);
    console.assert(v.m3 === -1234);
    console.assert(v.m4 === 54321);
    console.assert(v.m5 === -123456);
    console.assert(v.m6 === 3456789012);
    console.assert(v.m7 === -123456789n);
    console.assert(v.m8 === 9876543210n);
    console.assert(v.m9 === -10.25);
    console.assert(v.m10 === 100.5);
    console.assert(v.m11 === 'Ö');
    console.assert(v.m12 === 'hä');
    console.assert(v.m13.toString() === 'long');
    console.assert(v.m14 === -123456);
    console.assert(v.m15.length === 3);
    console.assert(v.m15[0] === 'foo');
    console.assert(v.m15[1] === 'barr');
    console.assert(v.m15[2] === 'bazzz');
    console.assert(v.m16 === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v.m17.m === -123456);
    console.assert(v.m18.m1.m === 'foo');
    console.assert(v.m18.m2 === -123456);
    console.assert(v.m18.m3 === -123456);
    console.assert(v.m18.m4.m === 'barr');
    console.assert(uno.sameUnoObject(v.m19, test));
    console.assert(test.isAnyStruct(
        new uno.idl.com.sun.star.testuno.Struct(
            {m1: true, m2: -12, m3: -1234, m4: 54321, m5: -123456, m6: 3456789012,
             m7: -123456789n, m8: 9876543210n, m9: -10.25, m10: 100.5, m11: 'Ö', m12: 'hä',
             m13: uno.type.long, m14: -123456, m15: ['foo', 'barr', 'bazzz'],
             m16: uno.idl.com.sun.star.testuno.Enum.E_2,
             m17: new uno.idl.com.sun.star.testuno.StructLong({m: -123456}),
             m18: new uno.idl.com.sun.star.testuno.Template(
                 [uno.type.any, uno.type.struct(uno.idl.com.sun.star.testuno.StructString)],
                 {m1: new uno.idl.com.sun.star.testuno.StructString({m: 'foo'}),
                  m2: -123456, m3: -123456,
                  m4: new uno.idl.com.sun.star.testuno.StructString({m: 'barr'})}),
             m19: test})));
    console.assert(test.isAnyStruct(
        new uno.Any(
            uno.type.struct(uno.idl.com.sun.star.testuno.Struct),
            {m1: true, m2: -12, m3: -1234, m4: 54321, m5: -123456, m6: 3456789012, m7: -123456789n,
             m8: 9876543210n, m9: -10.25, m10: 100.5, m11: 'Ö', m12: 'hä', m13: uno.type.long,
             m14: -123456, m15: ['foo', 'barr', 'bazzz'],
             m16: uno.idl.com.sun.star.testuno.Enum.E_2, m17: {m: -123456},
             m18: {m1: {m: 'foo'}, m2: -123456, m3: -123456, m4: {m: 'barr'}}, m19: test})));
}
)"
                       uR"(
{
    const v = test.getAnyException();
    console.log(v);
    console.assert(v.Message.startsWith('error'));
    console.assert(v.Context === null);
    console.assert(v.m1 === -123456);
    console.assert(v.m2 === 100.5);
    console.assert(v.m3 === 'hä');
    console.assert(test.isAnyException(
        new uno.idl.com.sun.star.testuno.Exception(
            {Message: 'error', m1: -123456, m2: 100.5, m3: 'hä'})));
    console.assert(test.isAnyException(
        new uno.Any(
            uno.type.exception(uno.idl.com.sun.star.testuno.Exception),
            {Message: 'error', Context: null, m1: -123456, m2: 100.5, m3: 'hä'})));
}
{
    const v = test.getAnyInterface();
    console.log(v);
    console.assert(uno.sameUnoObject(v, test));
    console.assert(test.isAnyInterface(
        new uno.Any(uno.type.interface(uno.idl.com.sun.star.testuno.XTest), test)));
}
{
    const v = test.getSequenceBoolean();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === true);
    console.assert(v[1] === true);
    console.assert(v[2] === false);
    console.assert(test.isSequenceBoolean([true, true, false]));
}
{
    const v = test.getSequenceByte();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -12);
    console.assert(v[1] === 1);
    console.assert(v[2] === 12);
    console.assert(test.isSequenceByte([-12, 1, 12]));
}
{
    const v = test.getSequenceShort();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -1234);
    console.assert(v[1] === 1);
    console.assert(v[2] === 1234);
    console.assert(test.isSequenceShort([-1234, 1, 1234]));
}
{
    const v = test.getSequenceUnsignedShort();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 1);
    console.assert(v[1] === 10);
    console.assert(v[2] === 54321);
    console.assert(test.isSequenceUnsignedShort([1, 10, 54321]));
}
{
    const v = test.getSequenceLong();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -123456);
    console.assert(v[1] === 1);
    console.assert(v[2] === 123456);
    console.assert(test.isSequenceLong([-123456, 1, 123456]));
}
{
    const v = test.getSequenceUnsignedLong();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 1);
    console.assert(v[1] === 10);
    console.assert(v[2] === 3456789012);
    console.assert(test.isSequenceUnsignedLong([1, 10, 3456789012]));
}
{
    const v = test.getSequenceHyper();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -123456789n);
    console.assert(v[1] === 1n);
    console.assert(v[2] === 123456789n);
    console.assert(test.isSequenceHyper([-123456789n, 1n, 123456789n]));
}
{
    const v = test.getSequenceUnsignedHyper();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 1n);
    console.assert(v[1] === 10n);
    console.assert(v[2] === 9876543210n);
    console.assert(test.isSequenceUnsignedHyper([1n, 10n, 9876543210n]));
}
{
    const v = test.getSequenceFloat();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -10.25);
    console.assert(v[1] === 1.5);
    console.assert(v[2] === 10.75);
    console.assert(test.isSequenceFloat([-10.25, 1.5, 10.75]));
}
{
    const v = test.getSequenceDouble();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -100.5);
    console.assert(v[1] === 1.25);
    console.assert(v[2] === 100.75);
    console.assert(test.isSequenceDouble([-100.5, 1.25, 100.75]));
}
{
    const v = test.getSequenceChar();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 'a');
    console.assert(v[1] === 'B');
    console.assert(v[2] === 'Ö');
    console.assert(test.isSequenceChar(['a', 'B', 'Ö']));
}
{
    const v = test.getSequenceString();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === 'foo');
    console.assert(v[1] === 'barr');
    console.assert(v[2] === 'bazzz');
    console.assert(test.isSequenceString(['foo', 'barr', 'bazzz']));
}
{
    const v = test.getSequenceType();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0].toString() === 'long');
    console.assert(v[1].toString() === 'void');
    console.assert(v[2].toString() === '[]com.sun.star.testuno.Enum');
    console.assert(test.isSequenceType([
        uno.type.long, uno.type.void,
        uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum))]));
}
{
    let v = test.getSequenceAny();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === -123456);
    console.assert(v[1] === undefined);
    console.assert(v[2].length === 3);
    console.assert(v[2][0] === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[2][1] === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[2][2] === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(test.isSequenceAny([
        -123456, undefined,
        new uno.Any(
            uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)),
            [uno.idl.com.sun.star.testuno.Enum.E_2,
             uno.idl.com.sun.star.testuno.Enum.E3,
             uno.idl.com.sun.star.testuno.Enum.E_10])]));
}
{
    const v = test.getSequenceSequenceString();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0].length === 0);
    console.assert(v[1].length === 2);
    console.assert(v[1][0] === 'foo');
    console.assert(v[1][1] === 'barr');
    console.assert(v[2].length === 1);
    console.assert(v[2][0] === 'baz');
    console.assert(test.isSequenceSequenceString([[], ['foo', 'barr'], ['baz']]));
}
{
    const v = test.getSequenceEnum();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0] === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[1] === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[2] === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(test.isSequenceEnum([
        uno.idl.com.sun.star.testuno.Enum.E_2, uno.idl.com.sun.star.testuno.Enum.E3,
        uno.idl.com.sun.star.testuno.Enum.E_10]));
}
)"
                       uR"(
{
    const v = test.getSequenceStruct();
    console.log(v);
    console.assert(v.length === 3);
    console.assert(v[0].m1 === true);
    console.assert(v[0].m2 === -12);
    console.assert(v[0].m3 === -1234);
    console.assert(v[0].m4 === 1);
    console.assert(v[0].m5 === -123456);
    console.assert(v[0].m6 === 1);
    console.assert(v[0].m7 === -123456789n);
    console.assert(v[0].m8 === 1n);
    console.assert(v[0].m9 === -10.25);
    console.assert(v[0].m10 === -100.5);
    console.assert(v[0].m11 === 'a');
    console.assert(v[0].m12 === 'hä');
    console.assert(v[0].m13.toString() === 'long');
    console.assert(v[0].m14 === -123456);
    console.assert(v[0].m15.length === 0);
    console.assert(v[0].m16 === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[0].m17.m === -123456);
    console.assert(v[0].m18.m1.m === 'foo');
    console.assert(v[0].m18.m2 === -123456);
    console.assert(v[0].m18.m3 === -123456);
    console.assert(v[0].m18.m4.m === 'barr');
    console.assert(uno.sameUnoObject(v[0].m19, test));
    console.assert(v[1].m1 === true);
    console.assert(v[1].m2 === 1);
    console.assert(v[1].m3 === 1);
    console.assert(v[1].m4 === 10);
    console.assert(v[1].m5 === 1);
    console.assert(v[1].m6 === 10);
    console.assert(v[1].m7 === 1n);
    console.assert(v[1].m8 === 10n);
    console.assert(v[1].m9 === 1.5);
    console.assert(v[1].m10 === 1.25);
    console.assert(v[1].m11 === 'B');
    console.assert(v[1].m12 === 'barr');
    console.assert(v[1].m13.toString() === 'void');
    console.assert(v[1].m14 === undefined);
    console.assert(v[1].m15.length === 2);
    console.assert(v[1].m15[0] === 'foo');
    console.assert(v[1].m15[1] === 'barr');
    console.assert(v[1].m16 === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[1].m17.m === 1);
    console.assert(v[1].m18.m1.m === 'baz');
    console.assert(v[1].m18.m2 === 1);
    console.assert(v[1].m18.m3 === undefined);
    console.assert(v[1].m18.m4.m === 'foo');
    console.assert(v[1].m19 === null);
    console.assert(v[2].m1 === false);
    console.assert(v[2].m2 === 12);
    console.assert(v[2].m3 === 1234);
    console.assert(v[2].m4 === 54321);
    console.assert(v[2].m5 === 123456);
    console.assert(v[2].m6 === 3456789012);
    console.assert(v[2].m7 === 123456789n);
    console.assert(v[2].m8 === 9876543210n);
    console.assert(v[2].m9 === 10.75);
    console.assert(v[2].m10 === 100.75);
    console.assert(v[2].m11 === 'Ö');
    console.assert(v[2].m12 === 'bazzz');
    console.assert(v[2].m13.toString() === '[]com.sun.star.testuno.Enum');
    console.assert(v[2].m14.length === 3);
    console.assert(v[2].m14[0] === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[2].m14[1] === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[2].m14[2] === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(v[2].m15.length === 1);
    console.assert(v[2].m15[0] === 'baz');
    console.assert(v[2].m16 === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(v[2].m17.m === 123456);
    console.assert(v[2].m18.m1.m === 'barr');
    console.assert(v[2].m18.m2 === 123456);
    console.assert(v[2].m18.m3.length === 3);
    console.assert(v[2].m18.m3[0] === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[2].m18.m3[1] === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[2].m18.m3[2] === uno.idl.com.sun.star.testuno.Enum.E_10);
    console.assert(v[2].m18.m4.m === 'bazz');
    console.assert(uno.sameUnoObject(v[2].m19, test));
    console.assert(test.isSequenceStruct([
        {m1: true, m2: -12, m3: -1234, m4: 1, m5: -123456, m6: 1, m7: -123456789n, m8: 1n,
         m9: -10.25, m10: -100.5, m11: 'a', m12: 'hä', m13: uno.type.long, m14: -123456, m15: [],
         m16: uno.idl.com.sun.star.testuno.Enum.E_2, m17: {m: -123456},
         m18: {m1: {m: 'foo'}, m2: -123456, m3: -123456, m4: {m: 'barr'}}, m19: test},
        {m1: true, m2: 1, m3: 1, m4: 10, m5: 1, m6: 10, m7: 1n, m8: 10n, m9: 1.5, m10: 1.25,
         m11: 'B', m12: 'barr', m13: uno.type.void, m14: undefined, m15: ['foo', 'barr'],
         m16: uno.idl.com.sun.star.testuno.Enum.E3, m17: {m: 1},
         m18: {m1: {m: 'baz'}, m2: 1, m3: undefined, m4: {m: 'foo'}}, m19: null},
        {m1: false, m2: 12, m3: 1234, m4: 54321, m5: 123456, m6: 3456789012, m7: 123456789n,
         m8: 9876543210n, m9: 10.75, m10: 100.75, m11: 'Ö', m12: 'bazzz',
         m13: uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)),
         m14: new uno.Any(
                  uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)),
                  [uno.idl.com.sun.star.testuno.Enum.E_2, uno.idl.com.sun.star.testuno.Enum.E3,
                   uno.idl.com.sun.star.testuno.Enum.E_10]),
         m15: ['baz'], m16: uno.idl.com.sun.star.testuno.Enum.E_10, m17: {m: 123456},
         m18: {m1: {m: 'barr'}, m2: 123456,
               m3: new uno.Any(
                   uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)),
                   [uno.idl.com.sun.star.testuno.Enum.E_2, uno.idl.com.sun.star.testuno.Enum.E3,
                    uno.idl.com.sun.star.testuno.Enum.E_10]),
               m4: {m: 'bazz'}},
         m19: test}]));
}
{
    const v = test.getNull();
    console.log(v);
    console.assert(v === null);
    console.assert(test.isNull(null));
}
{
    const v1 = {};
    const v2 = {};
    const v3 = {};
    const v4 = {};
    const v5 = {};
    const v6 = {};
    const v7 = {};
    const v8 = {};
    const v9 = {};
    const v10 = {};
    const v11 = {};
    const v12 = {};
    const v13 = {};
    const v14 = {};
    const v15 = {};
    const v16 = {};
    const v17 = {};
    const v18 = {};
    test.getOut(
        v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18);
    console.assert(v1.val === true);
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
    console.assert(v14.val === -123456)
    console.assert(v15.val.length === 3);
    console.assert(v15.val[0] === 'foo');
    console.assert(v15.val[1] === 'barr');
    console.assert(v15.val[2] === 'bazzz');
    console.assert(v16.val === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v17.val.m1 === true);
    console.assert(v17.val.m2 === -12);
    console.assert(v17.val.m3 === -1234);
    console.assert(v17.val.m4 === 54321);
    console.assert(v17.val.m5 === -123456);
    console.assert(v17.val.m6 === 3456789012);
    console.assert(v17.val.m7 === -123456789n);
    console.assert(v17.val.m8 === 9876543210n);
    console.assert(v17.val.m9 === -10.25);
    console.assert(v17.val.m10 === 100.5);
    console.assert(v17.val.m11 === 'Ö');
    console.assert(v17.val.m12 === 'hä');
    console.assert(v17.val.m13.toString() === 'long');
    console.assert(v17.val.m14 === -123456);
    console.assert(v17.val.m15.length === 3);
    console.assert(v17.val.m15[0] === 'foo');
    console.assert(v17.val.m15[1] === 'barr');
    console.assert(v17.val.m15[2] === 'bazzz');
    console.assert(v17.val.m16 === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v17.val.m17.m === -123456);
    console.assert(v17.val.m18.m1.m === 'foo');
    console.assert(v17.val.m18.m2 === -123456);
    console.assert(v17.val.m18.m3 === -123456);
    console.assert(v17.val.m18.m4.m === 'barr');
    console.assert(uno.sameUnoObject(v17.val.m19, test));
    console.assert(uno.sameUnoObject(v18.val, test));
}
)"
                       uR"(
try {
    test.throwRuntimeException();
    console.assert(false);
} catch (e) {
    console.assert(e instanceof uno.idl.com.sun.star.uno.RuntimeException);
    console.assert(e.Message.startsWith('test'));
}
try {
    throw new uno.idl.com.sun.star.lang.WrappedTargetException(
        {Message: 'wrapped', Context: test,
         TargetException: new uno.idl.com.sun.star.uno.RuntimeException({
             Message: 'test', Context: test})});
        console.assert(false);
} catch (e) {
    console.assert(e instanceof uno.idl.com.sun.star.lang.WrappedTargetException);
    console.assert(e.Message.startsWith('wrapped'));
    console.assert(uno.sameUnoObject(e.Context, test));
    console.assert(e.TargetException instanceof uno.idl.com.sun.star.uno.RuntimeException);
    console.assert(e.TargetException.Message.startsWith('test'));
    console.assert(uno.sameUnoObject(e.TargetException.Context, test));
}
console.assert(test.StringAttribute === 'hä');
test.StringAttribute = 'foo';
console.assert(test.StringAttribute === 'foo');
{
    const s = uno.idl.com.sun.star.testuno.theSingleton(uno.componentContext);
    const v = s.getString();
    console.assert(v === 'this is a string from XStringFactory');
}
{
    const s = uno.idl.com.sun.star.testuno.ImplicitConstructor.create(uno.componentContext);
    const v = s.getArguments();
    console.assert(v.length === 0);
}
{
    const s = uno.idl.com.sun.star.testuno.ExplicitConstructors.multipleArguments(
        uno.componentContext, -123456, 'hä', -10.25);
    const v = s.getArguments();
    console.assert(v.length === 3);
    console.assert(v[0] === -123456);
    console.assert(v[1] === 'hä');
    console.assert(v[2] === -10.25);
}
{
    const s = uno.idl.com.sun.star.testuno.ExplicitConstructors.interfaceArgument(
        uno.componentContext, test);
    const v = s.getArguments();
    console.assert(v.length === 1);
    console.assert(uno.sameUnoObject(v[0], test));
}
)"
                       uR"(
{
    const s = uno.idl.com.sun.star.testuno.ExplicitConstructors.restArgument(
        uno.componentContext, -123456, undefined,
        new uno.Any(
            uno.type.sequence(uno.type.enum(uno.idl.com.sun.star.testuno.Enum)),
            [uno.idl.com.sun.star.testuno.Enum.E_2,
             uno.idl.com.sun.star.testuno.Enum.E3,
             uno.idl.com.sun.star.testuno.Enum.E_10]));;
    const v = s.getArguments();
    console.assert(v.length === 3);
    console.assert(v[0] === -123456);
    console.assert(v[1] === undefined);
    console.assert(v[2].length === 3);
    console.assert(v[2][0] === uno.idl.com.sun.star.testuno.Enum.E_2);
    console.assert(v[2][1] === uno.idl.com.sun.star.testuno.Enum.E3);
    console.assert(v[2][2] === uno.idl.com.sun.star.testuno.Enum.E_10);
}

)"_ustr);
    }

    CPPUNIT_TEST_SUITE(TestUno);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUno);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
