/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cli_cs_multi.cs,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:24:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


