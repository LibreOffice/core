/*************************************************************************
 *
 *  $RCSfile: notesuno.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:49 $
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

#ifndef SC_NOTESUNO_HXX
#define SC_NOTESUNO_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"           // ScRange, ScAddress
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _COM_SUN_STAR_SHEET_XSHEETANNOTATION_HPP_
#include <com/sun/star/sheet/XSheetAnnotation.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XSIMPLETEXT_HPP_
#include <com/sun/star/text/XSimpleText.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif


class ScDocShell;
class SvxUnoText;


class ScAnnotationObj : public cppu::WeakImplHelper4<
                            com::sun::star::container::XChild,
                            com::sun::star::text::XSimpleText,
                            com::sun::star::sheet::XSheetAnnotation,
                            com::sun::star::lang::XServiceInfo >,
                        public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    SvxUnoText*             pUnoText;

private:
    SvxUnoText& GetUnoText();

public:
                            ScAnnotationObj(ScDocShell* pDocSh, const ScAddress& rPos);
    virtual                 ~ScAnnotationObj();

    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            // XChild
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                            getParent() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setParent( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::uno::XInterface >& Parent )
                                    throw(::com::sun::star::lang::NoSupportException,
                                            ::com::sun::star::uno::RuntimeException);

                            // XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL
                            createTextCursorByRange( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& aTextPosition )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertString( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        const ::rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   insertControlCharacter( const ::com::sun::star::uno::Reference<
                                        ::com::sun::star::text::XTextRange >& xRange,
                                        sal_Int16 nControlCharacter, sal_Bool bAbsorb )
                                    throw(::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL
                            getText() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL
                            getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL   setString( const ::rtl::OUString& aString )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XSheetAnnotation
    virtual ::com::sun::star::table::CellAddress SAL_CALL getPosition()
                                    throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAuthor() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDate() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL getIsVisible() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setIsVisible( sal_Bool bIsVisible )
                                    throw(::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);
};


#endif

