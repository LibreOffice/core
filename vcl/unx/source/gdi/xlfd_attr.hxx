/*************************************************************************
 *
 *  $RCSfile: xlfd_attr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-19 08:31:46 $
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
#ifndef XLFD_ATTRIBUTE_HXX
#define XLFD_ATTRIBUTE_HXX

#ifndef _SALUNX_H
#include <salunx.h>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif


struct Attribute {

    const char*     mpName;
    unsigned short  mnLength;
    unsigned short  mnValue;
    unsigned short  mnFeature;
    String*         mpAnnotation;

    const char*     GetName() const
                            { return mpName;   }
    unsigned short  GetValue() const
                            { return mnValue;  }
    unsigned short  GetLength() const
                            { return mnLength; }
    Bool            HasFeature( unsigned short nFeature ) const
                             { return ((mnFeature & nFeature) != 0); }
    const String   &GetAnnotation() const
                            { return *mpAnnotation; }

    void            SetName( const char *p, int nLen );
    void            SetValue( unsigned short nIn )
                            { mnValue = nIn; }
    void            SetAnnotation( String *pString )
                            { mpAnnotation = pString; }
    void            SetFeature( unsigned short nFeature )
                            { mnFeature = nFeature; }
    void            TagFeature( unsigned short nFeature );

    int             Compare( const char *p, int nLen );
    Bool            ExactMatch( const char *p, int nLen );
    void            Release();
};


void
AppendAttribute( Attribute *pAttribute, ByteString &rString );


typedef String*(*AttributeAnnotatorT)(const Attribute &rAttribute);
extern "C" {
typedef rtl_TextEncoding(*AttributeClassifierT)(const char* pStr);
}

#define XLFD_FEATURE_NONE                0x0000
#define XLFD_FEATURE_NARROW              0x0001
#define XLFD_FEATURE_OL_GLYPH            0x0002
#define XLFD_FEATURE_OL_CURSOR           0x0004
#define XLFD_FEATURE_REDUNDANTSTYLE      0x0008
#define XLFD_FEATURE_APPLICATION_FONT    0x0010

#define XLFD_FEATURE_INTERFACE_FONT      0x0020
#define XLFD_FEATURE_INTERFACE_FONT_MEDQ 0x0040
#define XLFD_FEATURE_INTERFACE_FONT_HIGQ 0x0080

// ---------------------------------------------------------------------------
//
//
// manage global lists of Attributes
// since XListFonts does never list more than 64K fonts this storage does
// handle array size and indices with unsigned short values for low
// memory consumption
//
//
// ---------------------------------------------------------------------------

class AttributeStorage {

    private:

        Attribute*      mpList;
        unsigned short  mnSize;
        unsigned short  mnCount;
        unsigned short  mnLastmatch;
        unsigned short  mnDefaultValue;

        void            Enlarge();
                        AttributeStorage();

    public:

                        AttributeStorage( unsigned short nDefaultValue );
                        ~AttributeStorage();
        unsigned short  Insert( const char *pString, int nLength );
        Attribute*      Retrieve( unsigned short nIndex ) const ;
        void            AddClassification( Attribute *pClassification,
                                unsigned short nNum );
        void            AddClassification( AttributeClassifierT Classify );
        void            TagFeature( unsigned short nFeature );
        void            AddAnnotation( AttributeAnnotatorT Annotate );
        #ifdef DEBUG
        void            Dump();
        #endif
};


// ---------------------------------------------------------------------------
//
//
// Attribute provider is a frame for a set of AttributeStorages. For XLFD
// interpretation and efficient storage, AttributeStorages for foundry,
// family_name, weight_name, slant, setwidth_name, add_style_name and combined
// charset_registry and charset_encoding are used. pixel_size, point_size,
// resolution_x and resolution_y are stored as numbers. please note that this
// does not allow storage of matrix-enhanced fonts. spacing is stored as
// a char, since only the 'm', 'c' and 'p' types are defined.
//
//
// ---------------------------------------------------------------------------

typedef enum eXLFDAttributeT {
    eXLFDFoundry = 0,
    eXLFDFamilyName,
    eXLFDWeightName,
    eXLFDSlant,
    eXLFDSetwidthName,
    eXLFDAddstyleName,
    eXLFDCharset,
    eXLFDMaxEntry
};

typedef enum eDeviceT {
    eDeviceDontKnow = 0,
    eDevicePrinter,
    eDeviceDisplay
};

class AttributeProvider {

    private:

        AttributeStorage*   mpField[ eXLFDMaxEntry ];

        AttributeStorage*   GetField( eXLFDAttributeT eXLFDField )
                                    { return mpField[ eXLFDField]; }
        eDeviceT            meOutputDevice;
    public:

                            AttributeProvider( eDeviceT eOutputDevice );
                            ~AttributeProvider();

        void                AddClassification();
        void                AddAnnotation();
        void                TagFeature();
        #ifdef DEBUG
        void                Dump();
        #endif

        eDeviceT            GetDevice() const
                                    { return meOutputDevice; }

        // these are just shortcuts or proxies for the most common used
        // AttributeStorage functionality
        AttributeStorage*   GetFoundry()
                                    { return GetField(eXLFDFoundry); }
        AttributeStorage*   GetFamily()
                                    { return GetField(eXLFDFamilyName); }
        AttributeStorage*   GetWeight()
                                    { return GetField(eXLFDWeightName); }
        AttributeStorage*   GetSlant()
                                    { return GetField(eXLFDSlant); }
        AttributeStorage*   GetSetwidth()
                                    { return GetField(eXLFDSetwidthName); }
        AttributeStorage*   GetAddstyle()
                                    { return GetField(eXLFDAddstyleName); }
        AttributeStorage*   GetCharset()
                                    { return GetField(eXLFDCharset); }

        Attribute*      RetrieveFoundry( unsigned short nIndex )
                            { return GetFoundry()->Retrieve(nIndex); }
        Attribute*      RetrieveFamily( unsigned short nIndex )
                            { return GetFamily()->Retrieve(nIndex); }
        Attribute*      RetrieveWeight( unsigned short nIndex )
                            { return GetWeight()->Retrieve(nIndex); }
        Attribute*      RetrieveSlant( unsigned short nIndex )
                            { return GetSlant()->Retrieve(nIndex); }
        Attribute*      RetrieveSetwidth( unsigned short nIndex )
                            { return GetSetwidth()->Retrieve(nIndex); }
        Attribute*      RetrieveAddstyle( unsigned short nIndex )
                            { return GetAddstyle()->Retrieve(nIndex); }
        Attribute*      RetrieveCharset( unsigned short nIndex )
                            { return GetCharset()->Retrieve(nIndex); }

        unsigned short  InsertFoundry( const char *pString, int nLength )
                            { return GetFoundry()->Insert(pString, nLength); }
        unsigned short  InsertFamily( const char *pString, int nLength )
                            { return GetFamily()->Insert(pString, nLength); }
        unsigned short  InsertWeight( const char *pString, int nLength )
                            { return GetWeight()->Insert(pString, nLength); }
        unsigned short  InsertSlant( const char *pString, int nLength )
                            { return GetSlant()->Insert(pString, nLength); }
        unsigned short  InsertSetwidth( const char *pString, int nLength )
                            { return GetSetwidth()->Insert(pString, nLength);}
        unsigned short  InsertAddstyle( const char *pString, int nLength )
                            { return GetAddstyle()->Insert(pString, nLength);}
        unsigned short  InsertCharset( const char *pString, int nLength )
                            { return GetCharset()->Insert(pString, nLength); }
};

#endif /* XLFD_ATTRIBUTE_HXX */

