/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacharacters.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:45:21 $
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
#include "vbacharacters.hxx"

#include "vbaglobals.hxx"
#include "vbafont.hxx"


using namespace ::org::openoffice;
using namespace ::com::sun::star;

ScVbaCharacters::ScVbaCharacters( const uno::Reference< vba::XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const ScVbaPalette& dPalette, const uno::Reference< text::XSimpleText>& xRange,const css::uno::Any& Start, const css::uno::Any& Length, sal_Bool Replace  ) throw ( css::lang::IllegalArgumentException ) : ScVbaCharacters_BASE( xParent, xContext ), m_xSimpleText(xRange), m_aPalette( dPalette),  nLength(-1), nStart(1), bReplace( Replace )
{
    Start >>= nStart;
    if ( nStart < 1 )
        nStart = 1; // silently correct user error ( as ms )
    nStart--; // OOo is 0 based
    Length >>=nLength;
    uno::Reference< text::XTextCursor > xTextCursor( m_xSimpleText->createTextCursor(), uno::UNO_QUERY_THROW );
    xTextCursor->collapseToStart();
    if ( nStart )
        xTextCursor->goRight( nStart, sal_False );
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
    throw uno::RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Not Implemented") ), uno::Reference< XInterface >() );
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


rtl::OUString&
ScVbaCharacters::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaCharacters") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaCharacters::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("org.openoffice.excel.Characters" ) );
    }
    return aServiceNames;
}

