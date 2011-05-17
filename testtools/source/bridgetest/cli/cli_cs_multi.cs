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

namespace testtools { namespace bridgetest { namespace cli_cs {

public class Multi: unoidl.test.testtools.bridgetest.XMulti
{
    public Multi()
    {
    }

    public double att1
    {
        get { return _att1; }
        set { _att1 = value; }
    }

    public int fn11(int arg)
    {
        return 11 * arg;
    }

    public string fn12(string arg)
    {
        return "12" + arg;
    }

    public int fn21(int arg)
    {
        return 21 * arg;
    }

    public string fn22(string arg)
    {
        return "22" + arg;
    }

    public double att3
    {
        get { return _att3; }
        set { _att3 = value; }
    }

    public int fn31(int arg)
    {
        return 31 * arg;
    }

    public string fn32(string arg)
    {
        return "32" + arg;
    }

    public int fn33()
    {
        return 33;
    }

    public int fn41(int arg)
    {
        return 41 * arg;
    }

    public int fn61(int arg)
    {
        return 61 * arg;
    }

    public string fn62(string arg)
    {
        return "62" + arg;
    }

    public int fn71(int arg)
    {
        return 71 * arg;
    }

    public string fn72(string arg)
    {
        return "72" + arg;
    }

    public int fn73()
    {
        return 73;
    }

    private double _att1;
    private double _att3;
};

} } }


