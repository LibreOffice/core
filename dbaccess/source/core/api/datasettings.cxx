/*************************************************************************
 *
 *  $RCSfile: datasettings.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-11 11:18:11 $
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

#ifndef _DBA_CORE_DATASETTINGS_HXX_
#include "datasettings.hxx"
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBASHARED_STRINGCONSTANTS_HRC_
#include "stringconstants.hrc"
#endif
#ifndef _DBA_CORE_PROPERTYHELPER_HXX_
#include "propertyhelper.hxx"
#endif
#ifndef _DBA_CORE_REGISTRYHELPER_HXX_
#include "registryhelper.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::registry;
using namespace ::comphelper;
using namespace ::cppu;
using namespace dbaccess;
using namespace comphelper;

//==========================================================================
//= ODataSettings
//==========================================================================
//--------------------------------------------------------------------------
void ODataSettings::registerProperties()
{
    registerProperty(PROPERTY_FILTER, PROPERTY_ID_FILTER, PropertyAttribute::BOUND,
                    &m_sFilter, ::getCppuType(&m_sFilter));

    registerProperty(PROPERTY_ORDER, PROPERTY_ID_ORDER, PropertyAttribute::BOUND,
                    &m_sOrder, ::getCppuType(&m_sOrder));

    registerProperty(PROPERTY_APPLYFILTER, PROPERTY_ID_APPLYFILTER, PropertyAttribute::BOUND,
                    &m_bApplyFilter, ::getBooleanCppuType());

    registerProperty(PROPERTY_FONT, PROPERTY_ID_FONT, PropertyAttribute::BOUND,
                    &m_aFont, ::getCppuType(&m_aFont));

    registerMayBeVoidProperty(PROPERTY_ROW_HEIGHT, PROPERTY_ID_ROW_HEIGHT, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aRowHeight, ::getCppuType(static_cast<sal_Int32*>(NULL)));

    registerMayBeVoidProperty(PROPERTY_TEXTCOLOR, PROPERTY_ID_TEXTCOLOR, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
                    &m_aTextColor, ::getCppuType(static_cast<sal_Int32*>(NULL)));
}
//--------------------------------------------------------------------------
ODataSettings::ODataSettings(OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
{
    registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings::ODataSettings(const ODataSettings& _rSource, ::cppu::OBroadcastHelper& _rBHelper)
    :OPropertyContainer(_rBHelper)
    ,ODataSettings_Base(_rSource)
{
    registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base()
    : m_bApplyFilter(sal_False)
    ,m_aFont(::comphelper::getDefaultFont())
{
    //  registerProperties();
}

//--------------------------------------------------------------------------
ODataSettings_Base::ODataSettings_Base(const ODataSettings_Base& _rSource)
{
    //  registerProperties();

    m_sFilter = _rSource.m_sFilter;
    m_sOrder = _rSource.m_sOrder;
    m_bApplyFilter = _rSource.m_bApplyFilter;
    m_aFont = _rSource.m_aFont;
    m_aRowHeight = _rSource.m_aRowHeight;
    m_aTextColor = _rSource.m_aTextColor;
}

//--------------------------------------------------------------------------
void ODataSettings_Base::storeTo(const Reference< XRegistryKey >& _rxConfigLocation) const
{
    try
    {
        if (!_rxConfigLocation.is() || _rxConfigLocation->isReadOnly())
        {
            OSL_ASSERT("ODataSettings::storeTo : invalid config key (NULL or readonly) !");
            return;
        }

        writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FILTER, m_sFilter);
        writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_ORDER, m_sOrder);
        writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_APPLYFILTER, m_bApplyFilter);

        if (m_aFont.Name.getLength())
        {
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_NAME, m_aFont.Name);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_HEIGHT, m_aFont.Height);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WIDTH, m_aFont.Width);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_STYLENAME, m_aFont.StyleName);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_FAMILY, m_aFont.Family);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_CHARSET, m_aFont.CharSet);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_PITCH, m_aFont.Pitch);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_UNDERLINE, m_aFont.Underline);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_STRIKEOUT, m_aFont.Strikeout);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_KERNING, m_aFont.Kerning);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WORDLINEMODE, m_aFont.WordLineMode);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_TYPE, m_aFont.Type);

            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH, (sal_Int32)m_aFont.CharacterWidth);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WEIGHT, (sal_Int32)m_aFont.Weight);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_SLANT, (sal_Int32)m_aFont.Slant);
            writeOrDelete(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_ORIENTATION, (sal_Int32)m_aFont.Orientation);
                // TODO : find a method to store the character width/weight/slant/orientation ...
                // or wait 'til we're really configuration based and thus are able to write doubles
        }
        else
            deleteKey(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_ROOTNODE);


        if (m_aRowHeight.hasValue())
            writeValue(_rxConfigLocation, CONFIGKEY_DEFSET_ROW_HEIGHT, getINT32(m_aRowHeight));
        else
            deleteKey(_rxConfigLocation, CONFIGKEY_DEFSET_ROW_HEIGHT);
        if (m_aTextColor.hasValue())
            writeValue(_rxConfigLocation, CONFIGKEY_DEFSET_TEXTCOLOR, getINT32(m_aTextColor));
        else
            deleteKey(_rxConfigLocation, CONFIGKEY_DEFSET_TEXTCOLOR);
    }
    catch(InvalidRegistryException&)
    {
    }
}

//--------------------------------------------------------------------------
void ODataSettings_Base::loadFrom(const Reference< XRegistryKey >& _rxConfigLocation)
{
    try
    {
        if (!_rxConfigLocation.is())
        {
            OSL_ASSERT("ODataSettings::loadFrom : invalid config key (NULL or readonly) !");
            return;
        }

        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FILTER, m_sFilter);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_ORDER, m_sOrder);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_APPLYFILTER, m_bApplyFilter);

        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_NAME, m_aFont.Name);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_HEIGHT, m_aFont.Height);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WIDTH, m_aFont.Width);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_STYLENAME, m_aFont.StyleName);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_FAMILY, m_aFont.Family);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_CHARSET, m_aFont.CharSet);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_PITCH, m_aFont.Pitch);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_UNDERLINE, m_aFont.Underline);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_STRIKEOUT, m_aFont.Strikeout);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_KERNING, m_aFont.Kerning);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WORDLINEMODE, m_aFont.WordLineMode);
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_TYPE, m_aFont.Type);

        sal_Int32 nTemp;
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_CHARACTERWIDTH, nTemp);
        m_aFont.CharacterWidth = nTemp;
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_WEIGHT, nTemp);
        m_aFont.Weight = nTemp;
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_SLANT, nTemp);
        m_aFont.Slant = (FontSlant)nTemp;
        readValue(_rxConfigLocation, CONFIGKEY_DEFSET_FONT_ORIENTATION, nTemp);
        m_aFont.Orientation = nTemp;
            // TODO : find a method to store the character width/weight/slant/orientation ...
            // or wait 'til we're really configuration based and thus are able to write doubles


        if (readValue(_rxConfigLocation, CONFIGKEY_DEFSET_ROW_HEIGHT, nTemp))
            m_aRowHeight <<= nTemp;
        else
            m_aRowHeight.clear();

        if (readValue(_rxConfigLocation, CONFIGKEY_DEFSET_TEXTCOLOR, nTemp))
            m_aTextColor <<= nTemp;
        else
            m_aTextColor.clear();
    }
    catch(InvalidRegistryException&)
    {
    }
}

