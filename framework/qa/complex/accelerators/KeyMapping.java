/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

class IdentifierHashMap extends HashMap
{
    public void put(String sIdentifier, Short nCode)
    {
        super.put(sIdentifier, nCode);
    }
    public Short get(String sIdentifier)
    {
        return (Short)super.get(sIdentifier);
    }
}

class CodeHashMap extends HashMap
{
    public void put(Short nCode, String sIdentifier)
    {
        super.put(nCode, sIdentifier);
    }
    public String get(Short nCode)
    {
        return (String)super.get(nCode);
    }
}

public class KeyMapping
{
    private IdentifierHashMap aIdentifierHashMap;
    private CodeHashMap aCodeHashMap;

    public KeyMapping()
    {
        KeyIdentifierInfo[] aInfoMap = {
            new KeyIdentifierInfo("0", new Short(com.sun.star.awt.Key.NUM0)),
            new KeyIdentifierInfo("1", new Short(com.sun.star.awt.Key.NUM1)),
            new KeyIdentifierInfo("2", new Short(com.sun.star.awt.Key.NUM2)),
            new KeyIdentifierInfo("3", new Short(com.sun.star.awt.Key.NUM3)),
            new KeyIdentifierInfo("4", new Short(com.sun.star.awt.Key.NUM4)),
            new KeyIdentifierInfo("5", new Short(com.sun.star.awt.Key.NUM5)),
            new KeyIdentifierInfo("6", new Short(com.sun.star.awt.Key.NUM6)),
            new KeyIdentifierInfo("7", new Short(com.sun.star.awt.Key.NUM7)),
            new KeyIdentifierInfo("8", new Short(com.sun.star.awt.Key.NUM8)),
            new KeyIdentifierInfo("9", new Short(com.sun.star.awt.Key.NUM9)),
            new KeyIdentifierInfo("A", new Short(com.sun.star.awt.Key.A)),
            new KeyIdentifierInfo("B", new Short(com.sun.star.awt.Key.B)),
            new KeyIdentifierInfo("C", new Short(com.sun.star.awt.Key.C)),
            new KeyIdentifierInfo("D", new Short(com.sun.star.awt.Key.D)),
            new KeyIdentifierInfo("E", new Short(com.sun.star.awt.Key.E)),
            new KeyIdentifierInfo("F", new Short(com.sun.star.awt.Key.F)),
            new KeyIdentifierInfo("G", new Short(com.sun.star.awt.Key.G)),
            new KeyIdentifierInfo("H", new Short(com.sun.star.awt.Key.H)),
            new KeyIdentifierInfo("I", new Short(com.sun.star.awt.Key.I)),
            new KeyIdentifierInfo("J", new Short(com.sun.star.awt.Key.J)),
            new KeyIdentifierInfo("K", new Short(com.sun.star.awt.Key.K)),
            new KeyIdentifierInfo("L", new Short(com.sun.star.awt.Key.L)),
            new KeyIdentifierInfo("M", new Short(com.sun.star.awt.Key.M)),
            new KeyIdentifierInfo("N", new Short(com.sun.star.awt.Key.N)),
            new KeyIdentifierInfo("O", new Short(com.sun.star.awt.Key.O)),
            new KeyIdentifierInfo("P", new Short(com.sun.star.awt.Key.P)),
            new KeyIdentifierInfo("Q", new Short(com.sun.star.awt.Key.Q)),
            new KeyIdentifierInfo("R", new Short(com.sun.star.awt.Key.R)),
            new KeyIdentifierInfo("S", new Short(com.sun.star.awt.Key.S)),
            new KeyIdentifierInfo("T", new Short(com.sun.star.awt.Key.T)),
            new KeyIdentifierInfo("U", new Short(com.sun.star.awt.Key.U)),
            new KeyIdentifierInfo("V", new Short(com.sun.star.awt.Key.V)),
            new KeyIdentifierInfo("W", new Short(com.sun.star.awt.Key.W)),
            new KeyIdentifierInfo("X", new Short(com.sun.star.awt.Key.X)),
            new KeyIdentifierInfo("Y", new Short(com.sun.star.awt.Key.Y)),
            new KeyIdentifierInfo("Z", new Short(com.sun.star.awt.Key.Z)),
            new KeyIdentifierInfo("F1", new Short(com.sun.star.awt.Key.F1)),
            new KeyIdentifierInfo("F2", new Short(com.sun.star.awt.Key.F2)),
            new KeyIdentifierInfo("F3", new Short(com.sun.star.awt.Key.F3)),
            new KeyIdentifierInfo("F4", new Short(com.sun.star.awt.Key.F4)),
            new KeyIdentifierInfo("F5", new Short(com.sun.star.awt.Key.F5)),
            new KeyIdentifierInfo("F6", new Short(com.sun.star.awt.Key.F6)),
            new KeyIdentifierInfo("F7", new Short(com.sun.star.awt.Key.F7)),
            new KeyIdentifierInfo("F8", new Short(com.sun.star.awt.Key.F8)),
            new KeyIdentifierInfo("F9", new Short(com.sun.star.awt.Key.F9)),
            new KeyIdentifierInfo("F10", new Short(com.sun.star.awt.Key.F10)),
            new KeyIdentifierInfo("F11", new Short(com.sun.star.awt.Key.F11)),
            new KeyIdentifierInfo("F12", new Short(com.sun.star.awt.Key.F12)),
            new KeyIdentifierInfo("DOWN", new Short(com.sun.star.awt.Key.DOWN)),
            new KeyIdentifierInfo("UP", new Short(com.sun.star.awt.Key.UP)),
            new KeyIdentifierInfo("LEFT", new Short(com.sun.star.awt.Key.LEFT)),
            new KeyIdentifierInfo("RIGHT", new Short(com.sun.star.awt.Key.RIGHT)),
            new KeyIdentifierInfo("HOME", new Short(com.sun.star.awt.Key.HOME)),
            new KeyIdentifierInfo("END", new Short(com.sun.star.awt.Key.END)),
            new KeyIdentifierInfo("PAGEUP", new Short(com.sun.star.awt.Key.PAGEUP)),
            new KeyIdentifierInfo("PAGEDOWN", new Short(com.sun.star.awt.Key.PAGEDOWN)),
            new KeyIdentifierInfo("RETURN", new Short(com.sun.star.awt.Key.RETURN)),
            new KeyIdentifierInfo("ESCAPE", new Short(com.sun.star.awt.Key.ESCAPE)),
            new KeyIdentifierInfo("TAB", new Short(com.sun.star.awt.Key.TAB)),
            new KeyIdentifierInfo("BACKSPACE", new Short(com.sun.star.awt.Key.BACKSPACE)),
            new KeyIdentifierInfo("SPACE", new Short(com.sun.star.awt.Key.SPACE)),
            new KeyIdentifierInfo("INSERT", new Short(com.sun.star.awt.Key.INSERT)),
            new KeyIdentifierInfo("DELETE", new Short(com.sun.star.awt.Key.DELETE)),
            new KeyIdentifierInfo("ADD", new Short(com.sun.star.awt.Key.ADD)),
            new KeyIdentifierInfo("SUBTRACT", new Short(com.sun.star.awt.Key.SUBTRACT)),
            new KeyIdentifierInfo("MULTIPLY", new Short(com.sun.star.awt.Key.MULTIPLY)),
            new KeyIdentifierInfo("DIVIDE", new Short(com.sun.star.awt.Key.DIVIDE)),
            new KeyIdentifierInfo("CUT", new Short(com.sun.star.awt.Key.CUT)),
            new KeyIdentifierInfo("COPY", new Short(com.sun.star.awt.Key.COPY)),
            new KeyIdentifierInfo("PASTE", new Short(com.sun.star.awt.Key.PASTE)),
            new KeyIdentifierInfo("UNDO", new Short(com.sun.star.awt.Key.UNDO)),
            new KeyIdentifierInfo("REPEAT", new Short(com.sun.star.awt.Key.REPEAT))
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
        return (aIdentifierHashMap.get(sIdentifier)).shortValue();
    }

    public String mapCode2Identifier(short nCode)
    {
        return (String)aCodeHashMap.get(new Short(nCode));
    }
}
