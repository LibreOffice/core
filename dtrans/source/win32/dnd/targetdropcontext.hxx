/*************************************************************************
 *
 *  $RCSfile: targetdropcontext.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 14:39:07 $
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

#ifndef _TARGETDROPCONTEXT_HXX_
#define _TARGETDROPCONTEXT_HXX_

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETDROPCONTEXT_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetDropContext.hpp>
#endif

#include "target.hxx"

using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

class TargetDropContext: public WeakImplHelper1<XDropTargetDropContext>
{
    // calls to the functions of XDropTargetDropContext are delegated
    // to non-interface functions of m_pDropTarget
    DropTarget* m_pDropTarget;

    TargetDropContext();
    TargetDropContext( const TargetDropContext&);
    TargetDropContext &operator= ( const TargetDropContext&);
public:
    TargetDropContext( DropTarget* pTarget);
    ~TargetDropContext();


    // XDropTargetDragContext
    virtual void SAL_CALL acceptDrop( sal_Int8 dropOperation )
        throw( RuntimeException);
    virtual void SAL_CALL rejectDrop(  )
        throw( RuntimeException);


/*  virtual Sequence< DataFlavor > SAL_CALL getCurrentDataFlavors(  )
        throw( RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& df )
        throw( RuntimeException);
*/

    // XDropTargetDropContext (inherits XDropTargetDragContext)
    virtual void SAL_CALL dropComplete( sal_Bool success )
        throw(  RuntimeException);
};
#endif
