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
#ifndef _SVX_POSITIONALTABITEM_HXX
#define _SVX_POSITIONALTABITEM_HXX

#include <editeng/editengdllapi.h>  // Needed for EDITENG_DLLPUBLIC
#include <svl/poolitem.hxx>         // Needed for inheritence from 'SfxPoolItem'
#include <editeng/svxenum.hxx>      // Needed for enums

// class SvxPositionalTabItem --------------------------------------------------

#define cDfltPTabLeaderChar    (sal_Unicode(0x00))


/*  [Description]

    This item describes a Absolute Position Tab Character.
    This is documented in:
    http://msdn.microsoft.com/en-us/library/documentformat.openxml.wordprocessing.positionaltab.aspx
*/

class EDITENG_DLLPUBLIC SvxPositionalTabItem : public SfxPoolItem
{
private:
    SvxPositionalTabAlignment    m_eAlignment;
    mutable sal_Unicode          m_cLeaderCharacter;
    SvxPositionalTabBase         m_eRelativeTo;

public:
    TYPEINFO();

    explicit SvxPositionalTabItem( sal_uInt16 nWhich  );
    SvxPositionalTabItem( sal_uInt16 nWhich,
                          const SvxPositionalTabAlignment eAlignment,
                          const sal_Unicode cLeaderCharacter,
                          const SvxPositionalTabBase eRelativeTo );
    SvxPositionalTabItem( const SvxPositionalTabItem& rPositionalTabItem );
    ~SvxPositionalTabItem();

    // Assignment operator, equality operator (caution: expensive!)
    SvxPositionalTabItem& operator=( const SvxPositionalTabItem& rPositionalTabItem );

    // this is already included in SfxPoolItem declaration
    //int             operator!=( const SvxPositionalTabItem& rPositionalTabItem ) const
    //                  { return !( operator==( rPositionalTabItem ) ); }

    // "pure virtual Methods" from SfxPoolItem
    virtual int              operator==( const SfxPoolItem& ) const;

    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream&, sal_uInt16 ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;


    void                       SetAlignment(SvxPositionalTabAlignment eAlignment)   { m_eAlignment = eAlignment;    }
    SvxPositionalTabAlignment& GetAlignment()                                       { return m_eAlignment;          }
    SvxPositionalTabAlignment  GetAlignment() const                                 { return m_eAlignment;          }

    void                       SetLeaderCharacter(sal_Unicode cLeaderCharacter)     { m_cLeaderCharacter = cLeaderCharacter;    }
    sal_Unicode&               GetLeaderCharacter()                                 { return m_cLeaderCharacter;                }
    sal_Unicode                GetLeaderCharacter() const                           { return m_cLeaderCharacter;                }

    void                       SetRelativeTo(SvxPositionalTabBase eRelativeTo)      { m_eRelativeTo = eRelativeTo;  }
    SvxPositionalTabBase&      GetRelativeTo()                                      { return m_eRelativeTo;         }
    SvxPositionalTabBase       GetRelativeTo() const                                { return m_eRelativeTo;         }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
