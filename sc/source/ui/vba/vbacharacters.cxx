/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbacharacters.hxx"

#include "vbaglobals.hxx"
#include "vbafont.hxx"


using namespace ::ooo::vba;
using namespace ::com::sun::star;

ScVbaCharacters::ScVbaCharacters( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const ScVbaPalette& dPalette, const uno::Reference< text::XSimpleText>& xRange,const css::uno::Any& Start, const css::uno::Any& Length, sal_Bool Replace  ) throw ( css::lang::IllegalArgumentException ) : ScVbaCharacters_BASE( xParent, xContext ), m_xSimpleText(xRange), m_aPalette( dPalette),  nLength(-1), nStart(1), bReplace( Replace )
{
    Start >>= nStart;
    if ( nStart < 1 )
        nStart = 1; // silently correct user error ( as ms )
    nStart--; // OOo is 0 based
    Length >>=nLength;
    uno::Reference< text::XTextCursor > xTextCursor( m_xSimpleText->createTextCursor(), uno::UNO_QUERY_THROW );
    xTextCursor->collapseToStart();
    if ( nStart )
    {
        if ( ( nStart + 1 ) > m_xSimpleText->getString().getLength() )
            //nStart = m_xSimpleText->getString().getLength();
            xTextCursor->gotoEnd( false );
        xTextCursor->goRight( nStart, false );
    }
    if ( nLength < 0 ) // expand to end
        xTextCursor->gotoEnd( sal_True );
    else
        xTextCursor->goRight( nLength, sal_True );
    m_xTextRange.set( xTextCursor, uno::UNO_QUERY_THROW );

}

::rtl::OUString SAL_CALL
ScVbaCharacters::getCaption() throw (css::uno::RuntimeException)
{
    return m_xTextRange->getString();
}
void SAL_CALL
ScVbaCharacters::setCaption( const ::rtl::OUString& _caption ) throw (css::uno::RuntimeException)
{
    m_xTextRange->setString( _caption );

}

::sal_Int32 SAL_CALL
ScVbaCharacters::getCount() throw (css::uno::RuntimeException)
{
    return getCaption().getLength();
}

::rtl::OUString SAL_CALL
ScVbaCharacters::getText() throw (css::uno::RuntimeException)
{
    return getCaption();
}
void SAL_CALL
ScVbaCharacters::setText( const ::rtl::OUString& _text ) throw (css::uno::RuntimeException)
{
    setCaption( _text );
}
uno::Reference< excel::XFont > SAL_CALL
ScVbaCharacters::getFont() throw (css::uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( m_xTextRange, uno::UNO_QUERY_THROW );
    return uno::Reference< excel::XFont >( new ScVbaFont( this, mxContext, m_aPalette, xProps ) );
}
void SAL_CALL
ScVbaCharacters::setFont( const uno::Reference< excel::XFont >& /*_font*/ ) throw (css::uno::RuntimeException)
{
    // #TODO #FIXME needs implementation, or can't be done?
    throw uno::RuntimeException( ::rtl::OUString( "Not Implemented"), uno::Reference< XInterface >() );
}


// Methods
void SAL_CALL
ScVbaCharacters::Insert( const ::rtl::OUString& String ) throw (css::uno::RuntimeException)
{
    m_xSimpleText->insertString( m_xTextRange, String, bReplace );
}

void SAL_CALL
ScVbaCharacters::Delete(  ) throw (css::uno::RuntimeException)
{
    // #FIXME #TODO is this a bit suspect?, I wonder should the contents
    // of the cell be deleted from the parent ( range )
    m_xSimpleText->setString(rtl::OUString());
}


rtl::OUString
ScVbaCharacters::getServiceImplName()
{
    return rtl::OUString("ScVbaCharacters");
}

uno::Sequence< rtl::OUString >
ScVbaCharacters::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.Characters" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
