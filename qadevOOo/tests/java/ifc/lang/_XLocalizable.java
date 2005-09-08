/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XLocalizable.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:15:25 $
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
package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.Locale;
import com.sun.star.lang.XLocalizable;
import lib.Status;


public class _XLocalizable extends MultiMethodTest {

    public XLocalizable oObj;
    protected Locale initialLocale;

    public void _getLocale() {
        initialLocale = oObj.getLocale();
        tRes.tested("getLocale()", initialLocale != null);
    }

    public void _setLocale() {
        requiredMethod("getLocale()");

        String ro = (String) tEnv.getObjRelation("XLocalizable.ReadOnly");
        if (ro != null) {
            log.println(ro);
            tRes.tested("setLocale()", Status.skipped(true));
            return;
        }
        Locale newLocale = new Locale("de", "DE", "");
        oObj.setLocale(newLocale);

        Locale getLocale = oObj.getLocale();
        boolean res = ((getLocale.Country.equals(newLocale.Country)) &&
                      (getLocale.Language.equals(newLocale.Language)));

        if (!res) {
            log.println("Expected Language " + newLocale.Language +
                        " and Country " + newLocale.Country);
            log.println("Getting Language " + getLocale.Language +
                        " and Country " + getLocale.Country);
        }

        oObj.setLocale(initialLocale);
        tRes.tested("setLocale()", res);
    }
}