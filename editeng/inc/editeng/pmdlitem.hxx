/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_PMDLITEM_HXX
#define _SVX_PMDLITEM_HXX

// include ---------------------------------------------------------------

#include <svl/stritem.hxx>
#include <editeng/editengdllapi.h>

// class SvxPageModelItem ------------------------------------------------

/*
[Beschreibung]
Dieses Item enthaelt einen Namen einer Seitenvorlage.
*/

class EDITENG_DLLPUBLIC SvxPageModelItem : public SfxStringItem
{
private:
    sal_Bool bAuto;

public:
    inline SvxPageModelItem( sal_uInt16 nWh  );
    inline SvxPageModelItem( const String& rModel, sal_Bool bA /*= sal_False*/,
                             sal_uInt16 nWh  );
    inline SvxPageModelItem& operator=( const SvxPageModelItem& rModel );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText, const IntlWrapper * = 0 ) const;

    virtual sal_Bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );
    sal_Bool IsAuto() const { return bAuto; }
};

inline SvxPageModelItem::SvxPageModelItem( sal_uInt16 nWh )
    : bAuto( sal_False )
{
    SetWhich( nWh );
}

inline SvxPageModelItem::SvxPageModelItem( const String& rModel, sal_Bool bA,
                                           sal_uInt16 nWh ) :
    SfxStringItem( nWh, rModel ),
    bAuto( bA )
{}

inline SvxPageModelItem& SvxPageModelItem::operator=( const SvxPageModelItem& rModel )
{
    SetValue( rModel.GetValue() );
    return *this;
}

#endif

