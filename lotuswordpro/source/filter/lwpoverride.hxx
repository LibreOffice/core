/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005           Created
 ************************************************************************/

#ifndef _LWPOVERRIDE_HXX
#define _LWPOVERRIDE_HXX

#include "lwpobjid.hxx"
#include "lwptools.hxx"

class LwpObjectStream;

enum STATE
{
    STATE_OFF   = 0,
    STATE_ON    = 1,
    STATE_STYLE = 2
};

class LwpOverride
{
public:
    LwpOverride() : m_nValues(0), m_nOverride(0), m_nApply(0) {}

    virtual ~LwpOverride(){}

    virtual LwpOverride* clone() const = 0;

    virtual void Read(LwpObjectStream* pStrm) = 0;

    void ReadCommon(LwpObjectStream* pStrm);

    void Clear();

    void Override(sal_uInt16 nBits, STATE eState);

protected:
    LwpOverride(LwpOverride const& rOther);

private:
    LwpOverride& operator=(LwpOverride const& rOther); // not implemented

protected:
    sal_uInt16  m_nValues;
    sal_uInt16  m_nOverride;
    sal_uInt16  m_nApply;
};

/////////////////////////////////////////////////////////////////
class LwpTextLanguageOverride : public LwpOverride
{
public:
    LwpTextLanguageOverride() : m_nLanguage(0) {}

    virtual LwpTextLanguageOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

protected:
    LwpTextLanguageOverride(LwpTextLanguageOverride const& rOther);

private:
    LwpTextLanguageOverride& operator=(LwpTextLanguageOverride const& rOther); // not implemented

private:
    sal_uInt16  m_nLanguage;
};

/////////////////////////////////////////////////////////////////
class LwpTextAttributeOverride : public LwpOverride
{
public:
    LwpTextAttributeOverride() : m_nHideLevels(0), m_nBaseLineOffset(0) {}

    virtual LwpTextAttributeOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    inline sal_uInt16 GetHideLevels() const;

    sal_Bool IsHighLight();

protected:
    LwpTextAttributeOverride(LwpTextAttributeOverride const& rOther);

private:
    LwpTextAttributeOverride& operator=(LwpTextAttributeOverride const& rOther); // not implemented

private:
    enum{
    TAO_HIGHLIGHT = 0x08,
    };
    sal_uInt16  m_nHideLevels;
    sal_uInt32  m_nBaseLineOffset;
};

inline sal_uInt16 LwpTextAttributeOverride::GetHideLevels() const
{
    return m_nHideLevels;
}
/////////////////////////////////////////////////////////////////
class LwpKinsokuOptsOverride : public LwpOverride
{
public:
    LwpKinsokuOptsOverride() : m_nLevels(0) {}

    virtual LwpKinsokuOptsOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

protected:
    LwpKinsokuOptsOverride(LwpKinsokuOptsOverride const& rOther);

private:
    LwpKinsokuOptsOverride& operator=(LwpKinsokuOptsOverride const& rOther); // not implemented

private:
    sal_uInt16  m_nLevels;
};

/////////////////////////////////////////////////////////////////
class LwpBulletOverride : public LwpOverride
{
public:
    LwpBulletOverride() {m_bIsNull = sal_True;}

    virtual LwpBulletOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    inline LwpObjectID GetSilverBullet() const;

    void Override(LwpBulletOverride* pOther);

    inline sal_Bool IsSilverBulletOverridden();
    inline sal_Bool IsSkipOverridden();
    inline sal_Bool IsRightAlignedOverridden();
    inline sal_Bool IsSkip();
    inline sal_Bool IsRightAligned();
    inline sal_Bool IsEditable();


    void OverrideSilverBullet(LwpObjectID aID);
    void OverrideSkip(sal_Bool bOver);
    void OverrideRightAligned(sal_Bool bOver);

    inline void RevertSilverBullet();
    inline void RevertSkip();
    inline void RevertRightAligned();

    sal_Bool IsInValid(){return m_bIsNull;}

protected:
    LwpBulletOverride(LwpBulletOverride const& rOther);

private:
    LwpBulletOverride& operator=(LwpBulletOverride const& rOther); // not implemented

private:
    enum
    {
        BO_SILVERBULLET = 0x01,
        // 0x02 is free
        BO_RIGHTALIGN   = 0x04,
        BO_EDITABLE     = 0x08,
        BO_SKIP         = 0x10
    };

    LwpObjectID m_SilverBullet;
    sal_Bool m_bIsNull;
};

inline LwpObjectID LwpBulletOverride::GetSilverBullet() const
{
    return m_SilverBullet;
}

inline sal_Bool LwpBulletOverride::IsSilverBulletOverridden()
{
    return (sal_Bool)((m_nOverride & BO_SILVERBULLET) != 0);
}

inline sal_Bool LwpBulletOverride::IsSkipOverridden()
{
    return (sal_Bool)((m_nOverride & BO_SKIP) != 0);
}

inline sal_Bool LwpBulletOverride::IsRightAlignedOverridden()
{
    return (sal_Bool)((m_nOverride & BO_RIGHTALIGN) != 0);
}

inline sal_Bool LwpBulletOverride::IsSkip()
{
    return (sal_Bool)((m_nValues & BO_SKIP) != 0);
}

inline sal_Bool LwpBulletOverride::IsEditable()
{
    return (sal_Bool)((m_nValues & BO_EDITABLE) != 0);
}

inline sal_Bool LwpBulletOverride::IsRightAligned()
{
    return (sal_Bool)((m_nValues & BO_RIGHTALIGN) != 0);
}

inline void LwpBulletOverride::RevertSilverBullet()
{
    LwpOverride::Override(BO_SILVERBULLET, STATE_STYLE);
}

inline void LwpBulletOverride::RevertSkip()
{
    LwpOverride::Override(BO_SKIP, STATE_STYLE);
}

inline void LwpBulletOverride::RevertRightAligned()
{
    LwpOverride::Override(BO_RIGHTALIGN, STATE_STYLE);
}

/////////////////////////////////////////////////////////////////
class LwpAlignmentOverride : public LwpOverride
{
public:
    LwpAlignmentOverride() : m_nAlignType(ALIGN_LEFT), m_nPosition(0), m_nAlignChar(0){}

    virtual LwpAlignmentOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    enum AlignType
    {
        ALIGN_LEFT          = 0,
        ALIGN_RIGHT         = 1,
        ALIGN_CENTER        = 2,
        ALIGN_JUSTIFY       = 3,
        ALIGN_JUSTIFYALL    = 4,
        ALIGN_NUMERICLEFT   = 5,
        ALIGN_NUMERICRIGHT  = 6,
        ALIGN_SQUEEZE       = 7
    };

    AlignType GetAlignType(){ return m_nAlignType; }
    void Override(LwpAlignmentOverride* other);//add by  1-24
    void OverrideAlignment(AlignType val);//add by  1-24

protected:
    LwpAlignmentOverride(LwpAlignmentOverride const& rOther);

private:
    LwpAlignmentOverride& operator=(LwpAlignmentOverride const& rOther); // not implemented

private:
    enum
    {
        AO_TYPE     = 0x01,
        AO_POSITION = 0x02,
        AO_CHAR     = 0x04
    };

    AlignType   m_nAlignType;
    sal_uInt32  m_nPosition;
    sal_uInt16  m_nAlignChar;
};

/////////////////////////////////////////////////////////////////
class LwpSpacingCommonOverride : public LwpOverride
{
public:
    LwpSpacingCommonOverride() : m_nSpacingType(SPACING_NONE), m_nAmount(0), m_nMultiple(65536){}

    virtual LwpSpacingCommonOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    enum SpacingType
    {
        SPACING_DYNAMIC = 0,
        SPACING_LEADING = 1,
        SPACING_CUSTOM  = 2,
        SPACING_NONE    = 3
    };

    SpacingType GetType() const {return m_nSpacingType;}
    sal_Int32 GetAmount() const {return m_nAmount;}
    sal_Int32 GetMultiple() const {return m_nMultiple;}

    void Override(LwpSpacingCommonOverride* other);
    void OverrideType(SpacingType val);
    void OverrideAmount(sal_Int32 val);
    void OverrideMultiple(sal_Int32 val);

protected:
    LwpSpacingCommonOverride(LwpSpacingCommonOverride const& rOther);

private:
    LwpSpacingCommonOverride& operator=(LwpSpacingCommonOverride const& rOther); // not implemented

protected:
    enum
    {
        SPO_TYPE    = 0x01,
        SPO_AMOUNT  = 0x02,
        SPO_MULTIPLE= 0x04
    };
    SpacingType m_nSpacingType;//sal_uInt16
    sal_Int32   m_nAmount;
    sal_Int32   m_nMultiple;
};

/////////////////////////////////////////////////////////////////
class LwpSpacingOverride : public LwpOverride
{
public:
    LwpSpacingOverride();
    virtual ~LwpSpacingOverride();

    virtual LwpSpacingOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    void Override(LwpSpacingOverride* other);

    LwpSpacingCommonOverride* GetSpacing(){return m_pSpacing;}
    LwpSpacingCommonOverride* GetAboveLineSpacing(){return m_pAboveLineSpacing;}
    LwpSpacingCommonOverride* GetAboveSpacing(){return m_pParaSpacingAbove;}
    LwpSpacingCommonOverride* GetBelowSpacing(){return m_pParaSpacingBelow;}

protected:
    LwpSpacingOverride(LwpSpacingOverride const& rOther);

private:
    LwpSpacingOverride& operator=(LwpSpacingOverride const& rOther); // not implemented

private:
    LwpSpacingCommonOverride*   m_pSpacing;
    LwpSpacingCommonOverride*   m_pAboveLineSpacing;
    LwpSpacingCommonOverride*   m_pParaSpacingAbove;
    LwpSpacingCommonOverride*   m_pParaSpacingBelow;
};

/////////////////////////////////////////////////////////////////
class LwpIndentOverride : public LwpOverride
{
public:
    LwpIndentOverride() : m_nAll(0), m_nFirst(0), m_nRest(0), m_nRight(0) {}

    virtual LwpIndentOverride* clone() const;

    void Read(LwpObjectStream* pStrm);

    enum
    {
        RELATIVE_FIRST, RELATIVE_REST, RELATIVE_ALL
    };

    inline double GetFirst() const;
    inline double GetLeft() const;
    inline double GetRight() const;

    sal_uInt16 GetRelative();
    sal_Bool IsUseRelative();
    void Override(LwpIndentOverride* other);
    void OverrideIndentAll(sal_Int32 val);
    void OverrideIndentFirst(sal_Int32 val);
    void OverrideIndentRight(sal_Int32 val);
    void OverrideIndentRest(sal_Int32 val);
    void OverrideUseRelative(sal_Bool use);
    void OverrideRelative(sal_uInt16 relative);
    sal_Int32 GetMAll() const {return m_nAll;}
    sal_Int32 GetMFirst() const {return m_nFirst;}
    sal_Int32 GetMRest() const {return m_nRest;}
    sal_Int32 GetMRight() const {return m_nRight;}
    void SetMAll(sal_Int32 val){m_nAll=val;}
    void SetMFirst(sal_Int32 val){m_nFirst=val;}
    void SetMRest(sal_Int32 val){m_nRest=val;}
    void SetMRight(sal_Int32 val){m_nRight=val;}

protected:
    LwpIndentOverride(LwpIndentOverride const& rOther);

private:
    LwpIndentOverride& operator=(LwpIndentOverride const& rOther); // not implemented

private:
    enum
    {
        IO_ALL          = 0x0001,
        IO_FIRST        = 0x0002,
        IO_REST         = 0x0004,
        IO_RIGHT        = 0x0008,
        IO_HANGING      = 0x0010,
        IO_EQUAL        = 0x0020,
        IO_BODY         = 0x0040,
        IO_REL_ALL      = 0x0080,
        IO_REL_FIRST    = 0x0100,
        IO_REL_REST     = 0x0200,
        IO_REL_FLAGS    = (IO_REL_ALL | IO_REL_FIRST | IO_REL_REST),
        IO_USE_RELATIVE = 0x0400
    };

    sal_Int32   m_nAll;
    sal_Int32   m_nFirst;
    sal_Int32   m_nRest;
    sal_Int32   m_nRight;
};

inline double LwpIndentOverride::GetFirst() const
{
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nFirst-m_nRest));
}
inline double LwpIndentOverride::GetLeft() const
{
        return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nAll+m_nRest));
}
inline double LwpIndentOverride::GetRight() const
{
    return LwpTools::ConvertToMetric(LwpTools::ConvertFromUnits(m_nRight));
}

/////////////////////////////////////////////////////////////////
class LwpBackgroundStuff;
class LwpAmikakeOverride : public LwpOverride
{
public:
    LwpAmikakeOverride();

    virtual ~LwpAmikakeOverride();

    virtual LwpAmikakeOverride* clone() const;

    void Read(LwpObjectStream* pStrm);
    enum
    {
        AMIKAKE_NONE        = 0,
        AMIKAKE_BACKGROUND  = 1,
        AMIKAKE_CHARACTER   = 2
    };

protected:
    LwpAmikakeOverride(LwpAmikakeOverride const& rOther);

private:
    LwpAmikakeOverride& operator=(LwpAmikakeOverride const& rOther); // not implemented

private:
    LwpBackgroundStuff* m_pBackgroundStuff;
    sal_uInt16      m_nType;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
