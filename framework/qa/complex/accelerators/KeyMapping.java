/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package complex.accelerators;

import java.util.HashMap;

class KeyIdentifierInfo
{
    protected String sIdentifier;
    protected Short nCode;

    KeyIdentifierInfo(String sID, Short nC)
    {
        sIdentifier = sID;
        nCode = nC;
    }
}

class IdentifierHashMap extends HashMap<String,Short>
{
}

class CodeHashMap extends HashMap<Short,String>
{
}

public class KeyMapping
{
    private final IdentifierHashMap aIdentifierHashMap;
    private final CodeHashMap aCodeHashMap;

    public KeyMapping()
    {
        KeyIdentifierInfo[] aInfoMap = {
            new KeyIdentifierInfo("0", Short.valueOf(com.sun.star.awt.Key.NUM0)),
            new KeyIdentifierInfo("1", Short.valueOf(com.sun.star.awt.Key.NUM1)),
            new KeyIdentifierInfo("2", Short.valueOf(com.sun.star.awt.Key.NUM2)),
            new KeyIdentifierInfo("3", Short.valueOf(com.sun.star.awt.Key.NUM3)),
            new KeyIdentifierInfo("4", Short.valueOf(com.sun.star.awt.Key.NUM4)),
            new KeyIdentifierInfo("5", Short.valueOf(com.sun.star.awt.Key.NUM5)),
            new KeyIdentifierInfo("6", Short.valueOf(com.sun.star.awt.Key.NUM6)),
            new KeyIdentifierInfo("7", Short.valueOf(com.sun.star.awt.Key.NUM7)),
            new KeyIdentifierInfo("8", Short.valueOf(com.sun.star.awt.Key.NUM8)),
            new KeyIdentifierInfo("9", Short.valueOf(com.sun.star.awt.Key.NUM9)),
            new KeyIdentifierInfo("A", Short.valueOf(com.sun.star.awt.Key.A)),
            new KeyIdentifierInfo("B", Short.valueOf(com.sun.star.awt.Key.B)),
            new KeyIdentifierInfo("C", Short.valueOf(com.sun.star.awt.Key.C)),
            new KeyIdentifierInfo("D", Short.valueOf(com.sun.star.awt.Key.D)),
            new KeyIdentifierInfo("E", Short.valueOf(com.sun.star.awt.Key.E)),
            new KeyIdentifierInfo("F", Short.valueOf(com.sun.star.awt.Key.F)),
            new KeyIdentifierInfo("G", Short.valueOf(com.sun.star.awt.Key.G)),
            new KeyIdentifierInfo("H", Short.valueOf(com.sun.star.awt.Key.H)),
            new KeyIdentifierInfo("I", Short.valueOf(com.sun.star.awt.Key.I)),
            new KeyIdentifierInfo("J", Short.valueOf(com.sun.star.awt.Key.J)),
            new KeyIdentifierInfo("K", Short.valueOf(com.sun.star.awt.Key.K)),
            new KeyIdentifierInfo("L", Short.valueOf(com.sun.star.awt.Key.L)),
            new KeyIdentifierInfo("M", Short.valueOf(com.sun.star.awt.Key.M)),
            new KeyIdentifierInfo("N", Short.valueOf(com.sun.star.awt.Key.N)),
            new KeyIdentifierInfo("O", Short.valueOf(com.sun.star.awt.Key.O)),
            new KeyIdentifierInfo("P", Short.valueOf(com.sun.star.awt.Key.P)),
            new KeyIdentifierInfo("Q", Short.valueOf(com.sun.star.awt.Key.Q)),
            new KeyIdentifierInfo("R", Short.valueOf(com.sun.star.awt.Key.R)),
            new KeyIdentifierInfo("S", Short.valueOf(com.sun.star.awt.Key.S)),
            new KeyIdentifierInfo("T", Short.valueOf(com.sun.star.awt.Key.T)),
            new KeyIdentifierInfo("U", Short.valueOf(com.sun.star.awt.Key.U)),
            new KeyIdentifierInfo("V", Short.valueOf(com.sun.star.awt.Key.V)),
            new KeyIdentifierInfo("W", Short.valueOf(com.sun.star.awt.Key.W)),
            new KeyIdentifierInfo("X", Short.valueOf(com.sun.star.awt.Key.X)),
            new KeyIdentifierInfo("Y", Short.valueOf(com.sun.star.awt.Key.Y)),
            new KeyIdentifierInfo("Z", Short.valueOf(com.sun.star.awt.Key.Z)),
            new KeyIdentifierInfo("F1", Short.valueOf(com.sun.star.awt.Key.F1)),
            new KeyIdentifierInfo("F2", Short.valueOf(com.sun.star.awt.Key.F2)),
            new KeyIdentifierInfo("F3", Short.valueOf(com.sun.star.awt.Key.F3)),
            new KeyIdentifierInfo("F4", Short.valueOf(com.sun.star.awt.Key.F4)),
            new KeyIdentifierInfo("F5", Short.valueOf(com.sun.star.awt.Key.F5)),
            new KeyIdentifierInfo("F6", Short.valueOf(com.sun.star.awt.Key.F6)),
            new KeyIdentifierInfo("F7", Short.valueOf(com.sun.star.awt.Key.F7)),
            new KeyIdentifierInfo("F8", Short.valueOf(com.sun.star.awt.Key.F8)),
            new KeyIdentifierInfo("F9", Short.valueOf(com.sun.star.awt.Key.F9)),
            new KeyIdentifierInfo("F10", Short.valueOf(com.sun.star.awt.Key.F10)),
            new KeyIdentifierInfo("F11", Short.valueOf(com.sun.star.awt.Key.F11)),
            new KeyIdentifierInfo("F12", Short.valueOf(com.sun.star.awt.Key.F12)),
            new KeyIdentifierInfo("DOWN", Short.valueOf(com.sun.star.awt.Key.DOWN)),
            new KeyIdentifierInfo("UP", Short.valueOf(com.sun.star.awt.Key.UP)),
            new KeyIdentifierInfo("LEFT", Short.valueOf(com.sun.star.awt.Key.LEFT)),
            new KeyIdentifierInfo("RIGHT", Short.valueOf(com.sun.star.awt.Key.RIGHT)),
            new KeyIdentifierInfo("HOME", Short.valueOf(com.sun.star.awt.Key.HOME)),
            new KeyIdentifierInfo("END", Short.valueOf(com.sun.star.awt.Key.END)),
            new KeyIdentifierInfo("PAGEUP", Short.valueOf(com.sun.star.awt.Key.PAGEUP)),
            new KeyIdentifierInfo("PAGEDOWN", Short.valueOf(com.sun.star.awt.Key.PAGEDOWN)),
            new KeyIdentifierInfo("RETURN", Short.valueOf(com.sun.star.awt.Key.RETURN)),
            new KeyIdentifierInfo("ESCAPE", Short.valueOf(com.sun.star.awt.Key.ESCAPE)),
            new KeyIdentifierInfo("TAB", Short.valueOf(com.sun.star.awt.Key.TAB)),
            new KeyIdentifierInfo("BACKSPACE", Short.valueOf(com.sun.star.awt.Key.BACKSPACE)),
            new KeyIdentifierInfo("SPACE", Short.valueOf(com.sun.star.awt.Key.SPACE)),
            new KeyIdentifierInfo("INSERT", Short.valueOf(com.sun.star.awt.Key.INSERT)),
            new KeyIdentifierInfo("DELETE", Short.valueOf(com.sun.star.awt.Key.DELETE)),
            new KeyIdentifierInfo("ADD", Short.valueOf(com.sun.star.awt.Key.ADD)),
            new KeyIdentifierInfo("SUBTRACT", Short.valueOf(com.sun.star.awt.Key.SUBTRACT)),
            new KeyIdentifierInfo("MULTIPLY", Short.valueOf(com.sun.star.awt.Key.MULTIPLY)),
            new KeyIdentifierInfo("DIVIDE", Short.valueOf(com.sun.star.awt.Key.DIVIDE)),
            new KeyIdentifierInfo("CUT", Short.valueOf(com.sun.star.awt.Key.CUT)),
            new KeyIdentifierInfo("COPY", Short.valueOf(com.sun.star.awt.Key.COPY)),
            new KeyIdentifierInfo("PASTE", Short.valueOf(com.sun.star.awt.Key.PASTE)),
            new KeyIdentifierInfo("UNDO", Short.valueOf(com.sun.star.awt.Key.UNDO)),
            new KeyIdentifierInfo("REPEAT", Short.valueOf(com.sun.star.awt.Key.REPEAT))
        };

        aIdentifierHashMap = new IdentifierHashMap();
        aCodeHashMap = new CodeHashMap();
        for (int i = 0; i<aInfoMap.length; i++)
        {
            aIdentifierHashMap.put(aInfoMap[i].sIdentifier, aInfoMap[i].nCode);
            aCodeHashMap.put(aInfoMap[i].nCode, aInfoMap[i].sIdentifier);
        }
    }

    public short mapIdentifier2Code(String sIdentifier)
    {
        return aIdentifierHashMap.get(sIdentifier).shortValue();
    }

    public String mapCode2Identifier(short nCode)
    {
        return aCodeHashMap.get(Short.valueOf(nCode));
    }
}
