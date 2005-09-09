/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextSectionsSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:27:56 $
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
package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextSectionsSupplier;


public class _XTextSectionsSupplier extends MultiMethodTest {
    public XTextSectionsSupplier oObj;

    public void _getTextSections() {
        XNameAccess sections = oObj.getTextSections();
        boolean res = checkSections(sections);
        tRes.tested("getTextSections()", res);
    }

    protected boolean checkSections(XNameAccess sections) {
        String[] sNames = sections.getElementNames();
        boolean res = true;

        for (int k = 0; k < sNames.length; k++) {
            try {
                res &= sections.hasByName(sNames[k]);
                res &= (sections.getByName(sNames[k]) != null);
                log.println("Works for ... " + sNames[k]);
            } catch (com.sun.star.container.NoSuchElementException e) {
                log.println("positive test failed " + e.getMessage());
                res = false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("positive test failed " + e.getMessage());
                res = false;
            }
        }

        try {
            sections.getByName("unknown");
            log.println("negative test failed ... no Exception thrown");
            res = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected Exception for wrong argument ... OK");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("negative test failed ... wrong Exception thrown");
            res = false;
        }

        return res;
    }
}