/*************************************************************************
 *
 *  $RCSfile: richtextunowrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-07 16:13:47 $
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

#ifndef FORMS_SOURCE_RICHTEXT_RICHTEXTUNOWRAPPER_HXX
#include "richtextunowrapper.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
/** === end UNO includes === **/

#ifndef _SVX_UNOFORED_HXX
#include <svx/unofored.hxx>
#endif
#ifndef _MyEDITVIEW_HXX
#include <svx/editview.hxx>
#endif

//........................................................................
namespace frm
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;

    //====================================================================
    namespace
    {
        const SfxItemPropertyMap* getTextEnginePropertyMap()
        {
            // Propertymap fuer einen Outliner Text
            static const SfxItemPropertyMap aTextEnginePropertyMap[] =
            {
                SVX_UNOEDIT_CHAR_PROPERTIES,
                SVX_UNOEDIT_FONT_PROPERTIES,
                SVX_UNOEDIT_PARA_PROPERTIES,
                { MAP_CHAR_LEN("TextUserDefinedAttributes"), EE_CHAR_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { MAP_CHAR_LEN("ParaUserDefinedAttributes"), EE_PARA_XMLATTRIBS, &::getCppuType( static_cast< const Reference< XNameContainer >* >( NULL ) ), 0, 0 },
                { NULL, NULL }
            };

            return aTextEnginePropertyMap;
        }
    }

    //====================================================================
    //= ORichTextUnoWrapper
    //====================================================================
    //--------------------------------------------------------------------
    ORichTextUnoWrapper::ORichTextUnoWrapper( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :SvxUnoText( getTextEnginePropertyMap() )
    {
        SetEditSource( new RichTextEditSource( _rEngine, _pTextChangeListener ) );
    }

    //--------------------------------------------------------------------
    ORichTextUnoWrapper::~ORichTextUnoWrapper() throw()
    {
    }

    //====================================================================
    //= RichTextEditSource
    //====================================================================
    //--------------------------------------------------------------------
    RichTextEditSource::RichTextEditSource( EditEngine& _rEngine, IEngineTextChangeListener* _pTextChangeListener )
        :m_rEngine              ( _rEngine                               )
        ,m_pTextForwarder       ( new SvxEditEngineForwarder( _rEngine ) )
        ,m_pTextChangeListener  ( _pTextChangeListener                   )
    {
    }

    //--------------------------------------------------------------------
    RichTextEditSource::~RichTextEditSource()
    {
        delete m_pTextForwarder;
    }

    //--------------------------------------------------------------------
    SvxEditSource* RichTextEditSource::Clone() const
    {
        return new RichTextEditSource( m_rEngine, m_pTextChangeListener );
    }

    //--------------------------------------------------------------------
    SvxTextForwarder* RichTextEditSource::GetTextForwarder()
    {
        return m_pTextForwarder;
    }

    //--------------------------------------------------------------------
    void RichTextEditSource::UpdateData()
    {
        // this means that the content of the EditEngine changed via the UNO API
        // to reflect this in the views, we need to update them
        USHORT viewCount = m_rEngine.GetViewCount();
        for ( USHORT view = 0; view < viewCount; ++view )
        {
            EditView* pView = m_rEngine.GetView( view );
            if ( pView )
                pView->ForceUpdate();
        }

        if ( m_pTextChangeListener )
            m_pTextChangeListener->potentialTextChange();
    }

//........................................................................
}  // namespace frm
//........................................................................

