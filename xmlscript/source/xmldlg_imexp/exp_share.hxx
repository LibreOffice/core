/*************************************************************************
 *
 *  $RCSfile: exp_share.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-20 14:05:25 $
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
#include <vector>

#include <xmlscript/xmldlg_imexp.hxx>

#include <osl/diagnose.h>
#include <cppuhelper/implbase1.hxx>

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
    short _set;

    OUString _id;

    Style()
        : _set( 0 )
        {}

    Reference< xml::sax::XAttributeList > createElement();
};
class StyleBag
{
    vector< Style * > _styles;
public:
    ~StyleBag();

    OUString getStyleId( Style const & rStyle )
        throw ();

    void dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut );
};

class ElementDescriptor
    : public ::cppu::WeakImplHelper1< xml::sax::XAttributeList >
{
    Reference< beans::XPropertySet > _xProps;
    Reference< beans::XPropertyState > _xPropState;

    OUString _name;
    vector< OUString > _attrNames;
    vector< OUString > _attrValues;

    vector< Reference< xml::sax::XAttributeList > > _subElems;

public:
    inline ElementDescriptor(
        Reference< beans::XPropertySet > const & xProps,
        Reference< beans::XPropertyState > const & xPropState,
        OUString const & name )
        throw ()
        : _xProps( xProps )
        , _xPropState( xPropState )
        , _name( name )
        {}
    inline ElementDescriptor(
        OUString const & name )
        throw ()
        : _name( name )
        {}
    //
    inline OUString getName()
        { return _name; }
    //
    void addSubElem( Reference< xml::sax::XAttributeList > const & xElem );
    inline Reference< xml::sax::XAttributeList > getSubElemAt( sal_Int32 nIndex )
        { return _subElems[ nIndex ]; }
    void dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut );
    //
    Any readProp( OUString const & rPropName );
    //
    void readDefaults();
    //
    void readStringAttr( OUString const & rPropName, OUString const & rAttrName );
    void readLongAttr( OUString const & rPropName, OUString const & rAttrName );
    void readHexLongAttr( OUString const & rPropName, OUString const & rAttrName );
    void readShortAttr( OUString const & rPropName, OUString const & rAttrName );
    void readBoolAttr( OUString const & rPropName, OUString const & rAttrName );
    void readAlignAttr( OUString const & rPropName, OUString const & rAttrName );
    //
    inline void addAttr( OUString const & rAttrName, OUString const & rValue );
    inline void addBoolAttr( OUString const & rAttrName, sal_Bool bValue );

    //
    void readButtonModel( StyleBag * all_styles )
        throw (Exception);
    void readEditModel( StyleBag * all_styles )
        throw (Exception);
    void readCheckBoxModel( StyleBag * all_styles )
        throw (Exception);
    void readRadioButtonModel( StyleBag * all_styles )
        throw (Exception);
    void readComboBoxModel( StyleBag * all_styles )
        throw (Exception);
    void readCurrencyFieldModel( StyleBag * all_styles )
        throw (Exception);
    void readDateFieldModel( StyleBag * all_styles )
        throw (Exception);
    void readFileControlModel( StyleBag * all_styles )
        throw (Exception);
    void readFixedTextModel( StyleBag * all_styles )
        throw (Exception);
    void readGroupBoxModel( StyleBag * all_styles )
        throw (Exception);
    void readImageControlModel( StyleBag * all_styles )
        throw (Exception);
    void readListBoxModel( StyleBag * all_styles )
        throw (Exception);
    void readNumericFieldModel( StyleBag * all_styles )
        throw (Exception);
    void readPatternFieldModel( StyleBag * all_styles )
        throw (Exception);
    void readTimeFieldModel( StyleBag * all_styles )
        throw (Exception);
    void readContainerModel( StyleBag * all_styles )
        throw (Exception);

    // XAttributeList
    virtual sal_Int16 SAL_CALL getLength()
        throw (RuntimeException);
    virtual OUString SAL_CALL getNameByIndex( sal_Int16 nPos )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByIndex( sal_Int16 nPos )
        throw (RuntimeException);
    virtual OUString SAL_CALL getTypeByName( OUString const & rName )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByIndex( sal_Int16 nPos )
        throw (RuntimeException);
    virtual OUString SAL_CALL getValueByName( OUString const & rName )
        throw (RuntimeException);
};
//__________________________________________________________________________________________________
inline void ElementDescriptor::addAttr( OUString const & rAttrName, OUString const & rValue )
{
    _attrNames.push_back( rAttrName );
    _attrValues.push_back( rValue );
}
//__________________________________________________________________________________________________
inline void ElementDescriptor::addBoolAttr( OUString const & rAttrName, sal_Bool bValue )
{
    addAttr( rAttrName,
             (bValue
              ? OUString( RTL_CONSTASCII_USTRINGPARAM("true") )
              : OUString( RTL_CONSTASCII_USTRINGPARAM("false") )) );
}

//##################################################################################################


};
