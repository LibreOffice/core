/*************************************************************************
 *
 *  $RCSfile: staticmb.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: ganaya $ $Date: 2001-02-23 15:37:04 $
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

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase5.hxx>

#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XDictionary1.hpp>
#include <com/sun/star/linguistic2/XDictionaryEventListener.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/DictionaryEvent.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <com/sun/star/linguistic2/XPossibleHyphens.hpp>
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>

using namespace com::sun::star::linguistic2;
using namespace com::sun::star::frame;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace cppu {

    ClassData1 WeakImplHelper1< XDictionaryEntry >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XDictionaryEventListener >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XHyphenator >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XHyphenatedWord >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XPossibleHyphens >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XDictionaryListEventListener >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XSpellAlternatives >::s_aCD = ClassData1( 1 );
    ClassData1 WeakImplHelper1< XThesaurus >::s_aCD = ClassData1( 1 );

    ClassData2 WeakImplHelper2< XLinguServiceEventListener, XDictionaryListEventListener >::s_aCD = ClassData2( 1 );
    ClassData2 WeakImplHelper2< XSpellChecker, XSpellChecker1 >::s_aCD = ClassData2( 1 );

    ClassData3 WeakImplHelper3< XDictionary, XDictionaryEntry, XStorable >::s_aCD = ClassData3( 1 );
      ClassData3 WeakImplHelper3< XDictionary1, XDictionary, XStorable >::s_aCD = ClassData3( 1 );
      ClassData3 WeakImplHelper3< XSearchableDictionaryList, XComponent, XServiceInfo >::s_aCD = ClassData3( 1 );
      ClassData3 WeakImplHelper3< XLinguServiceManager, XComponent, XServiceInfo >::s_aCD = ClassData3( 1 );

      ClassData5 WeakImplHelper5< XPropertySet, XFastPropertySet, XPropertyAccess, XComponent, XServiceInfo >::s_aCD = ClassData5( 1 );

}

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;


namespace uno {

    typelib_TypeDescriptionReference * Sequence< XDictionaryEntry >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference< XDictionaryEntry > >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< DictionaryEvent >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference < XDictionary > >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference < XSpellChecker > >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference < XSpellChecker1 > >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference < XThesaurus > >::s_pType = 0;
    typelib_TypeDescriptionReference * Sequence< Reference < XMeaning > >::s_pType = 0;

}
