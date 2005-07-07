/*************************************************************************
 *
 *  $RCSfile: Wrapper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-07 13:17:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., September, 2004
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2004 by Sun Microsystems, Inc.
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.comp.beans;

import com.sun.star.uno.UnoRuntime;

//==========================================================================
/** Wrapper base class for UNO services which emulates the upcoming
     mode of automatic runtime Java classes to get rid of the need for
    queryInterface.

    Because its not worth the efford to create a runtime generated wrapper
    for this purpose, as it might be for OOo 2.0, you still have to use
    UnoRuntime.queryInterface() for interfaces which are optional or come
    from a subclass.  But for non optional interfaces you can already
    directly call their methods.

    This wrapper will only work for UNO objects via a bridge, not for
    direct Java objects.

    @since OOo 2.0.0
 */
class Wrapper
    implements
        com.sun.star.uno.IQueryInterface,
        com.sun.star.lang.XComponent
{
    private com.sun.star.uno.IQueryInterface xQueryInterface;
    private com.sun.star.lang.XComponent xComponent;

    public Wrapper( com.sun.star.uno.XInterface xProxy )
    {
        xQueryInterface = (com.sun.star.uno.IQueryInterface) xProxy;
        xComponent = (com.sun.star.lang.XComponent)
            UnoRuntime.queryInterface(
                com.sun.star.lang.XComponent.class, xProxy );
    }

    //==============================================================
    // com.sun.star.uno.IQueryInterface
    //--------------------------------------------------------------

    public String getOid()
    {
        return xQueryInterface.getOid();
    }

    public boolean isSame( Object aObject )
    {
        return xQueryInterface.isSame( aObject );
    }

    public Object queryInterface( com.sun.star.uno.Type aType )
    {
//System.err.println( "Wrapper::queryInterface(" + aType + ")" );
        return xQueryInterface.queryInterface( aType );
    }

    //==============================================================
    // com.sun.star.lang.XComponent
    //--------------------------------------------------------------

    public void dispose(  )
    {
        xComponent.dispose();
    }

    public void addEventListener( /*IN*/ com.sun.star.lang.XEventListener xListener )
    {
        xComponent.addEventListener( xListener );
    }

    public void removeEventListener( /*IN*/ com.sun.star.lang.XEventListener xListener )
    {
        xComponent.removeEventListener( xListener );
    }
};


