/*************************************************************************
 *
 *  $RCSfile: WeakComponentBase.cs,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2003-03-28 10:17:48 $
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

using System;
using unoidl.com.sun.star.lang;

namespace uno.util
{

/** This class can be used as a base class for UNO objects.
    It implements the capability to be kept weakly
    (unoidl.com.sun.star.uno.XWeak) and it implements
    unoidl.com.sun.star.lang.XTypeProvider which is necessary for
    using the object from StarBasic.
    In addition, it implements the interface
    unoidl.com.sun.star.lang.XComponent to be disposed explicitly.
*/
public class WeakComponentBase : WeakBase, XComponent, IDisposable
{
    private delegate void t_disposing( EventObject evt );
    private t_disposing m_disposing = null;
    
    /** Disposing callback called when the object is being disposed.
        This method has to be implemented by sub classes.
    */
    protected virtual void disposing()
    {
    }
    
    // XComponent impl
    /** This method is called by the owner of this object to explicitly
        dispose it.  This implementation of dispose() first notifies all
        registered event listeners and finally this object by calling its
        protected disposing().
    */
    public void dispose()
    {
        // send disposing notifications to listeners
        t_disposing call;
        lock (this)
        {
            call = m_disposing;
            m_disposing = null;
        }
        if (null != call)
        {
            EventObject evt = new EventObject( this );
            call( evt );
        }
        // call sub class
        disposing();
    }
    /** Registers an event listener being notified when this object is disposed.

        @param xListener event listener
    */
    public void addEventListener( XEventListener xListener )
    {
        lock (this)
        {
            m_disposing += new t_disposing( xListener.disposing );
        }
    }
    /** Revokes an event listener from being notified when this object is disposed.
        
        @param xListener event listener
    */
    public void removeEventListener( XEventListener xListener )
    {
        lock (this)
        {
            m_disposing -= new t_disposing( xListener.disposing );
        }
    }
    
    /** System.IDisposable implementation to conveniently auto dispose
        UNO objects within CLI code.
    */
    public void Dispose()
    {
        dispose();
    }
}

}
