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

ScVbaCharacters::ScVbaCharacters( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const ScVbaPalette& dPalette, const uno::Reference< text::XSimpleText>& xRange,const css::uno::Any& Start, const css::uno::Any& Length, bool Replace  ) throw ( css::lang::IllegalArgumentException, css::uno::RuntimeException ) : ScVbaCharacters_BASE( xParent, xContext ), m_xSimpleText(xRange), m_aPalette( dPalette),  nLength(-1), nStart(1), bReplace( Replace )
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
        xTextCursor->gotoEnd( true );
    else
        xTextCursor->goRight( nLength, true );
    m_xTextRange.set( xTextCursor, uno::UNO_QUERY_THROW );

}

OUString SAL_CALL
ScVbaCharacters::getCaption() throw (css::uno::RuntimeException, std::exception)
{
    return m_xTextRange->getString();
}
void SAL_CALL
ScVbaCharacters::setCaption( const OUString& _caption ) throw (css::uno::RuntimeException, std::exception)
{
    m_xTextRange->setString( _caption );

}

::sal_Int32 SAL_CALL
ScVbaCharacters::getCount() throw (css::uno::RuntimeException, std::exception)
{
    return getCaption().getLength();
}

OUString SAL_CALL
ScVbaCharacters::getText() throw (css::uno::RuntimeException, std::exception)
{
    return getCaption();
}
void SAL_CALL
ScVbaCharacters::setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception)
{
    setCaption( _text );
}
uno::Reference< excel::XFont > SAL_CALL
ScVbaCharacters::getFont() throw (css::uno::RuntimeException, std::exception)
{
    uno::Reference< beans::XPropertySet > xProps( m_xTextRange, uno::UNO_QUERY_THROW );
    return uno::Reference< excel::XFont >( new ScVbaFont( this, mxContext, m_aPalette, xProps ) );
}
void SAL_CALL
ScVbaCharacters::setFont( const uno::Reference< excel::XFont >& /*_font*/ ) throw (css::uno::RuntimeException, std::exception)
{
    // #TODO #FIXME needs implementation, or can't be done?
    throw uno::RuntimeException("Not Implemented" );
}

// Methods
void SAL_CALL
ScVbaCharacters::Insert( const OUString& rString ) throw (css::uno::RuntimeException, std::exception)
{
    m_xSimpleText->insertString( m_xTextRange, rString, bReplace );
}

void SAL_CALL
ScVbaCharacters::Delete(  ) throw (css::uno::RuntimeException, std::exception)
{
    // #FIXME #TODO is this a bit suspect?, I wonder should the contents
    // of the cell be deleted from the parent ( range )
    m_xSimpleText->setString(OUString());
}

OUString
ScVbaCharacters::getServiceImplName()
{
    return OUString("ScVbaCharacters");
}

uno::Sequence< OUString >
ScVbaCharacters::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.Characters";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
