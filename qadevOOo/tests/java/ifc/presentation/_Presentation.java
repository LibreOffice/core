/*************************************************************************
 *
 *  $RCSfile: _Presentation.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:48:18 $
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

package ifc.presentation;

import lib.MultiPropertyTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.presentation.XPresentation;

/**
* Testing <code>com.sun.star.presentation.Presentation</code>
* service properties :
* <ul>
*  <li><code> AllowAnimations</code></li>
*  <li><code> CustomShow</code></li>
*  <li><code> FirstPage</code></li>
*  <li><code> IsAlwaysOnTop</code></li>
*  <li><code> IsAutomatic</code></li>
*  <li><code> IsEndless</code></li>
*  <li><code> IsFullScreen</code></li>
*  <li><code> IsLivePresentation</code></li>
*  <li><code> IsMouseVisible</code></li>
*  <li><code> Pause</code></li>
*  <li><code> StartWithNavigator</code></li>
*  <li><code> UsePen</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Presentation'</code> (of type <code>XPresentation</code>):
*   for test of property <code>IsLivePresentation</code>
*   presentation start needed </li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.presentation.Presentation
* @see com.sun.star.presentation.XPresentation
*/
public class _Presentation extends MultiPropertyTest {

    public void _IsLivePresentation() {
        XPresentation aPresentation = (XPresentation)
            tEnv.getObjRelation("Presentation");
        if (aPresentation == null) throw new StatusException(Status.failed
            ("Relation 'Presentation' not found"));

        aPresentation.start();
        testProperty("IsLivePresentation");
    }

    protected PropertyTester CustomShowTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if ( ((String)oldValue).equals("SecondPresentation") ) {
                return new String("FirstPresentation");
            } else {
                return new String("SecondPresentation");
            }
        }
    };

    public void _CustomShow() {
        testProperty("CustomShow", CustomShowTester);
    }
}  // finish class _Presentation


