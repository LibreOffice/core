/*************************************************************************
 *
 *  $RCSfile: cli_cs_multi.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 13:29:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


