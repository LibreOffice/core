/*************************************************************************
 *
 *  $RCSfile: exp_share.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dbo $ $Date: 2001-04-04 14:35:09 $
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
#include <hash_map>

#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xml_helper.hxx>

#include <osl/diagnose.h>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>


using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

struct Style
{
    sal_uInt32 _backgroundColor;
    sal_uInt32 _textColor;
    sal_Int16 _border;
    awt::FontDescriptor _descr;
    short _all;
    short _set;

    OUString _id;

    inline Style( short all_ ) SAL_THROW( () )
        : _all( all_ )
        , _set( 0 )
        {}

    Reference< xml::sax::XAttributeList > createElement();
};
class StyleBag
{
    vector< Style * > _styles;

public:
    ~StyleBag() SAL_THROW( () );

    OUString getStyleId( Style const & rStyle ) SAL_THROW( () );

    void dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut );
};

class ElementDescriptor
    : public ::xmlscript::XMLElement
{
    Reference< beans::XPropertySet > _xProps;
    Reference< beans::XPropertyState > _xPropState;

public:
    inline ElementDescriptor(
        Reference< beans::XPropertySet > const & xProps,
        Reference< beans::XPropertyState > const & xPropState,
        OUString const & name )
        SAL_THROW( () )
        : XMLElement( name )
        , _xProps( xProps )
        , _xPropState( xPropState )
        {}
    inline ElementDescriptor(
        OUString const & name )
        SAL_THROW( () )
        : XMLElement( name )
        {}

    //
    Any readProp( OUString const & rPropName );
    //
    void readDefaults();
    //
    void readStringAttr( OUString const & rPropName, OUString const & rAttrName );
    void readDoubleAttr( OUString const & rPropName, OUString const & rAttrName );
    void readLongAttr( OUString const & rPropName, OUString const & rAttrName );
    void readHexLongAttr( OUString const & rPropName, OUString const & rAttrName );
    void readShortAttr( OUString const & rPropName, OUString const & rAttrName );
    void readBoolAttr( OUString const & rPropName, OUString const & rAttrName );
    void readAlignAttr( OUString const & rPropName, OUString const & rAttrName );
    void readDateFormatAttr( OUString const & rPropName, OUString const & rAttrName );
    void readTimeFormatAttr( OUString const & rPropName, OUString const & rAttrName );
    //
    inline void addBoolAttr( OUString const & rAttrName, sal_Bool bValue ) SAL_THROW( () );

    //
    void readEvents() SAL_THROW( (Exception) );
    //
    void readDialogModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readButtonModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readEditModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readCheckBoxModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readRadioButtonModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readComboBoxModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readCurrencyFieldModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readDateFieldModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readFileControlModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readFixedTextModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readGroupBoxModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readImageControlModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readListBoxModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readNumericFieldModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readPatternFieldModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
    void readTimeFieldModel( StyleBag * all_styles ) SAL_THROW( (Exception) );
};
//__________________________________________________________________________________________________
inline void ElementDescriptor::addBoolAttr( OUString const & rAttrName, sal_Bool bValue )
    SAL_THROW( () )
{
    addAttribute( rAttrName,
                  (bValue
                   ? OUString( RTL_CONSTASCII_USTRINGPARAM("true") )
                   : OUString( RTL_CONSTASCII_USTRINGPARAM("false") )) );
}

//##################################################################################################


};
