/*************************************************************************
 *
 *  $RCSfile: breakiteratorImpl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_BREAKITERATOR_HXX_
#define _I18N_BREAKITERATOR_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/XLocaleData.hpp>
#include <cppuhelper/implbase2.hxx> // helper for implementations

#include <tools/string.hxx>
#include <tools/list.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator
//  ----------------------------------------------------
class BreakIteratorImpl : public cppu::WeakImplHelper2
<
    XBreakIterator,
    com::sun::star::lang::XServiceInfo
>
{
public:
    BreakIteratorImpl( const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF );
    BreakIteratorImpl();
    ~BreakIteratorImpl();

    virtual sal_Int32 SAL_CALL nextCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);

    virtual Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL isBeginWord( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isEndWord( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getWordType( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL beginOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);

    virtual LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int16 SAL_CALL getScriptType( const rtl::OUString& Text, sal_Int32 nPos )
        throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL beginOfScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextScript( const rtl::OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL beginOfCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL previousCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextCharBlock( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 CharType ) throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    sal_Int16 SAL_CALL getScriptClass(sal_Unicode cChar );
    Boundary result; // for word break iterator

private :

    struct lookupTableItem {
        lookupTableItem(const com::sun::star::lang::Locale& _aLocale, com::sun::star::uno::Reference < XBreakIterator >& _xBI) : aLocale(_aLocale), xBI(_xBI) {};
        com::sun::star::lang::Locale aLocale;
        com::sun::star::uno::Reference < XBreakIterator > xBI;
    };
    List lookupTable;
    com::sun::star::lang::Locale aLocale;
    com::sun::star::uno::Reference < XBreakIterator > xBI;

    com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory > xMSF;
    sal_Bool SAL_CALL createLocaleSpecificBreakIterator( const rtl::OUString& aLocaleName )
        throw( com::sun::star::uno::RuntimeException );
    com::sun::star::uno::Reference < XBreakIterator > SAL_CALL getLocaleSpecificBreakIterator( const com::sun::star::lang::Locale& rLocale )
        throw( com::sun::star::uno::RuntimeException );
    const com::sun::star::lang::Locale& SAL_CALL getLocaleByScriptType(const com::sun::star::lang::Locale& rLocale, const rtl::OUString& Text,
        sal_Int32 nStartPos, sal_Bool forward, sal_Bool skipWhiteSpace)
        throw(com::sun::star::uno::RuntimeException);

};

} // i18n
} // star
} // sun
} // com


  /********************************************************************

    Source Code Control System - Updates

    $Log: not supported by cvs2svn $
    Revision 1.22  2001/11/12 19:34:11  er
    #94609# del: numberformatcode.hxx

    Revision 1.21  2001/10/19 21:16:43  bustamam.harun
    #84725# Add XServiceInfo implementation

    Revision 1.20  2001/09/24 14:59:50  jp
    script of character 0x1 and 0x2 are WEAK - this are our fields

    Revision 1.19  2001/04/12 23:53:54  bustamam.harun
    Fix compile problem on Solaris: change String to rtl::OUString

    Revision 1.18  2001/03/28 09:24:12  tl
    #84966 aEmptyPropValSeq member removed

    Revision 1.17  2000/12/05 15:22:58  jp
    Bug #81298#: CTOR with MultiServiceFactory

    Revision 1.16  2000/11/18 22:57:37  bustamam.harun
    change to ::i18n

    Revision 1.15  2000/10/29 17:01:45  er
    i18n API moved from com.sun.star.lang to com.sun.star.i18n

    Revision 1.14  2000/10/25 14:02:21  tl
    linguistic => linguistic2 : member of type PropertyValues as argument for hyphenate added

    Revision 1.13  2000/10/22 19:07:03  er
    SRC610 MUST: ::comphelper::getProcessServiceFactory

    Revision 1.12  2000/08/18 00:37:30  gmu
    new files for cjk breakiterators

    Revision 1.11  2000/08/11 12:41:46  jp
    Bug #77428#: change int to sal_Int32, change size of some static array

    Revision 1.10  2000/08/10 15:18:58  jp
    remove the not used methods queryInterface/acquire/release - because it's handled by the base class correct!

    Revision 1.9  2000/07/03 10:13:11  gmu
    added getScriptClass()

    Revision 1.8  2000/06/30 10:38:06  jp
    change the OWeak base class to the WeakImpHelper class

    Revision 1.7  2000/06/23 11:36:47  gmu
    changed API to reflect changes in IDL file

    Revision 1.6  2000/06/20 16:02:56  hr
    #includes with lowercase letters

    Revision 1.5  2000/06/14 13:32:28  sse
    Merged Begin and End word breaking

    Revision 1.4  2000/06/14 08:25:06  sse
    Updated hyphenator interface

    Revision 1.3  2000/06/13 08:13:00  sse
    new breakiterator API

    Revision 1.2  2000/06/09 10:10:59  sse
    Initial version of unicode breakiterator

    Revision 1.1  2000/06/07 12:22:43  mt
    BreakIterator


  *********************************************************************/


#endif // _I18N_BREAKITERATOR_HXX_
