/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docstoragemodifylistener.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 19:44:58 $
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

#ifndef SFX2_DOCSTORAGEMODIFYLISTENER_HXX
#define SFX2_DOCSTORAGEMODIFYLISTENER_HXX

#include "sfx2/dllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/util/XModifyListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase1.hxx>

//........................................................................
namespace sfx2
{
//........................................................................

    //====================================================================
    //= IModifiableDocument
    //====================================================================
    /** callback for the DocumentStorageModifyListener class
    */
    class SAL_NO_VTABLE IModifiableDocument
    {
    public:
        /// indicates the root or a sub storage of the document has been modified
        virtual void storageIsModified() = 0;
    };

    //====================================================================
    //= DocumentStorageModifyListener
    //====================================================================
    typedef ::cppu::WeakImplHelper1 < ::com::sun::star::util::XModifyListener > DocumentStorageModifyListener_Base;

    class SFX2_DLLPUBLIC DocumentStorageModifyListener : public DocumentStorageModifyListener_Base
    {
        ::osl::Mutex&           m_rMutex;
        IModifiableDocument*    m_pDocument;

    public:
        DocumentStorageModifyListener( ::osl::Mutex& _rMutex, IModifiableDocument& _rDocument );

        void dispose();

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        virtual ~DocumentStorageModifyListener();

    private:
        DocumentStorageModifyListener();                                                    // never implemented
        DocumentStorageModifyListener( const DocumentStorageModifyListener& );              // never implemented
        DocumentStorageModifyListener& operator=( const DocumentStorageModifyListener& );   // never implemented
    };

//........................................................................
} // namespace sfx2
//........................................................................

#endif // SFX2_DOCSTORAGEMODIFYLISTENER_HXX
