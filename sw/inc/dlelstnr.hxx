/*************************************************************************
 *
 *  $RCSfile: dlelstnr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tl $ $Date: 2000-10-27 11:43:40 $
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

#ifndef _DLELSTNR_HXX_
#define _DLELSTNR_HXX_

#include <cppuhelper/weak.hxx>

#ifndef _COM_SUN_STAR_LINGUISTIC_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLISTEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

///////////////////////////////////////////////////////////////////////////
// SwDicListEvtListener
// is a XDictionaryListEventListener that triggers spellchecking
// and hyphenation (yet to be implemented) when relevant changes to the
// dictionaries of the dictionary list were made.
//

class SwDicListEvtListener : public cppu::WeakImplHelper1
<
    ::com::sun::star::linguistic2::XDictionaryListEventListener
>
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >    xDicList;

    // disallow use of copy-constructor and assignment operator
    SwDicListEvtListener(const SwDicListEvtListener &);
    SwDicListEvtListener & operator = (const SwDicListEvtListener &);

public:
    SwDicListEvtListener(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryList >  &rxDicList );
    virtual ~SwDicListEvtListener();

    // XEventListener
    virtual void SAL_CALL disposing(
            const ::com::sun::star::lang::EventObject& rSource )
        throw(::com::sun::star::uno::RuntimeException);

    // XDictionaryListEventListener
    virtual void    SAL_CALL processDictionaryListEvent(
            const ::com::sun::star::linguistic2::DictionaryListEvent& aDicListEvent)
        throw( ::com::sun::star::uno::RuntimeException );
};


#endif

