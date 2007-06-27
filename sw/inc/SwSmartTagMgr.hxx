/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwSmartTagMgr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 13:12:31 $
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

#ifndef _SWSMARTTAGMGR_HXX
#define _SWSMARTTAGMGR_HXX

#ifndef _SMARTTAGMGR_HXX
#include <svx/SmartTagMgr.hxx>
#endif


/*************************************************************************
 *                      class SwSmartTagMgr
 *
 * Wrapper for the svx SmartTagMgr
 *************************************************************************/

class SwSmartTagMgr : public SmartTagMgr
{
private:
    static SwSmartTagMgr* mpTheSwSmartTagMgr;

    SwSmartTagMgr( const rtl::OUString& rModuleName );
    virtual ~SwSmartTagMgr();

public:
    static SwSmartTagMgr& Get();

    // ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::util::XChangesListener
      virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

/*
namespace SwSmartTagMgr
{
    SmartTagMgr& Get();
}
*/

#endif
