/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFX_FRMDESCRHXX
#define _SFX_FRMDESCRHXX

#include <bf_sfx2/sfxsids.hrc>

#include <bf_svtools/poolitem.hxx>
#include <tools/urlobj.hxx>
#include <tools/string.hxx>
#include <bf_svtools/svarray.hxx>
#include <tools/gen.hxx>
class SvStream;
class SvStrings;
class Wallpaper;
namespace binfilter {
class SfxItemSet;

struct SfxFrameDescriptor_Impl;
struct SfxFrameSetDescriptor_Impl;
class SfxFrameDescriptor;
typedef SfxFrameDescriptor* SfxFrameDescriptorPtr;
SV_DECL_PTRARR(SfxFramesArr, SfxFrameDescriptorPtr, 4, 2)//STRIP008 ;

//===========================================================================
// Die SfxFrame...descriptoren bauen eine rekursive Struktur auf, die
// s"amtliche zur Anzeige des Frame-Dokuments erforderlichen Daten umfa\st.
// "Uber einen SfxFrameSetDescriptor hat man Zugriff auf die gesamte darunter
// liegende Struktur.
// Aufgrund der besonderen Eigenschaften des SfxFrames-Dokuments ist ein
// SfxFramesSetDescriptor nicht nur der Inhalt dieses Dokuments, sondern
// beschreibt auch die View darauf.
// Das FrameSet wird aus Zeilen aufgebaut, die wiederum die eigentlichen
// Fenster enthalten. Eine Zeile kann horizontal oder vertikal ausgerichtet
// sein, woraus sich auch das Alignment des FrameSets ergibt.
//===========================================================================

enum ScrollingMode
{
    ScrollingYes,
    ScrollingNo,
    ScrollingAuto
};

enum SizeSelector
{
    SIZE_ABS,
    SIZE_PERCENT,
    SIZE_REL
};

#define BORDER_SET			2
#define BORDER_YES			1
#define BORDER_NO			0
#define SPACING_NOT_SET		-1L
#define SIZE_NOT_SET		-1L

class SfxFrameSetDescriptor
{
friend class SfxFrameDescriptor;

    SfxFramesArr 			aFrames;
    String					aBitmapName;
    SfxFrameSetDescriptor_Impl* pImp;
    String					aDocumentTitle;
    SfxFrameDescriptor*		pParentFrame;
    long					nFrameSpacing;
    USHORT					nHasBorder;
    USHORT					nMaxId;
    BOOL					bIsRoot;
    BOOL					bRowSet;
    SvStrings*				pScripts;
    SvStrings*				pComments;
};

struct SfxFrameProperties;

class SfxFrameDescriptor
{
friend class SfxFrameSetDescriptor;

    SfxFrameSetDescriptor* 	pParentFrameSet;    // Der Vater
    SfxFrameSetDescriptor*	pFrameSet;			// Wenn Kinder da sind, die
                                                // Set-Daten dazu
    INetURLObject			aURL;
    INetURLObject			aActualURL;
    String					aName;
    Size					aMargin;
    long					nWidth;
    ScrollingMode			eScroll;
    SizeSelector			eSizeSelector;
    USHORT					nHasBorder;
    USHORT					nItemId;
    BOOL					bResizeHorizontal;
    BOOL					bResizeVertical;
    BOOL					bHasUI;
    BOOL                    bReadOnly;
    SfxFrameDescriptor_Impl* pImp;
    SvStrings*				pScripts;
    SvStrings*				pComments;

public:
                            SfxFrameDescriptor(SfxFrameSetDescriptor* pSet);
                            ~SfxFrameDescriptor();

                            // Eigenschaften

                            // FileName/URL
    const INetURLObject&	GetURL() const
                            { return aURL; }
    void					SetURL( const String& rURL );
    const INetURLObject&	GetActualURL() const
                            { return aActualURL; }
    void                    SetReadOnly( BOOL bSet ) { bReadOnly = bSet;}
    BOOL                    IsReadOnly(  ) const { return bReadOnly;}

                            // Size
    void					SetWidth( long n )
                            { nWidth = n; }
    void					SetWidthPercent( long n )
                            { nWidth = n; eSizeSelector = SIZE_PERCENT; }
    void					SetWidthRel( long n )
                            { nWidth = n; eSizeSelector = SIZE_REL; }
    void					SetWidthAbs( long n )
                            { nWidth = n; eSizeSelector = SIZE_ABS; }
    long					GetWidth() const
                            { return nWidth; }
    SizeSelector			GetSizeSelector() const
                            { return eSizeSelector; }
    BOOL					IsResizable() const
                            { return bResizeHorizontal && bResizeVertical; }
    void					SetResizable( BOOL bRes )
                            { bResizeHorizontal = bResizeVertical = bRes; }

                            // FrameName
    const String&			GetName() const
                            { return aName; }
    void					SetName( const String& rName )
                            { aName = rName; }

                            // Margin, Scrolling
    const Size&				GetMargin() const
                            { return aMargin; }
    void					SetMargin( const Size& rMargin )
                            { aMargin = rMargin; }
    ScrollingMode			GetScrollingMode() const
                            { return eScroll; }
    void					SetScrollingMode( ScrollingMode eMode )
                            { eScroll = eMode; }

    BOOL					IsFrameBorderOn() const
                            { return ( nHasBorder & BORDER_YES ) != 0; }

    void					SetFrameBorder( BOOL bBorder )
                            {
                                nHasBorder = bBorder ?
                                            BORDER_YES | BORDER_SET :
                                            BORDER_NO | BORDER_SET;
                            }
    BOOL					IsFrameBorderSet() const
                            { return (nHasBorder & BORDER_SET) != 0; }
    void					ResetBorder()
                            { nHasBorder = 0; }

    BOOL					HasUI() const
                            { return bHasUI; }
    void					SetHasUI( BOOL bOn )
                            { bHasUI = bOn; }

                            // Attribute f"ur das Splitwindow
    USHORT					GetItemId() const
                            { return nItemId; }
    void					SetItemId( USHORT nId )
                            { nItemId = nId; }

                            // Rekursion, Iteration
    SfxFrameSetDescriptor*	GetParent() const
                            { return pParentFrameSet; }
    SfxFrameSetDescriptor*	GetFrameSet() const
                            { return pFrameSet; }
    void					SetFrameSet( SfxFrameSetDescriptor* pSet)
                            {
                                if ( pFrameSet )
                                    pFrameSet->pParentFrame = NULL;
                                pFrameSet = pSet;
                                if ( pSet )
                                    pSet->pParentFrame = this;
                            }

                            // Kopie z.B. f"ur die Views
    SfxFrameDescriptor*		Clone(SfxFrameSetDescriptor *pFrame=NULL, BOOL bWithIds = TRUE ) const;

};

// Kein Bock, einen operator= zu implementieren...
struct SfxFrameProperties
{
    String								aURL;
    String								aName;
    long								lMarginWidth;
    long								lMarginHeight;
    long								lSize;
    long								lSetSize;
    long								lFrameSpacing;
    long								lInheritedFrameSpacing;
    ScrollingMode						eScroll;
    SizeSelector						eSizeSelector;
    SizeSelector						eSetSizeSelector;
    BOOL								bHasBorder;
    BOOL								bBorderSet;
    BOOL								bResizable;
    BOOL								bSetResizable;
    BOOL								bIsRootSet;
    BOOL								bIsInColSet;
    BOOL								bHasBorderInherited;
    SfxFrameDescriptor*              	pFrame;

private:
    SfxFrameProperties( SfxFrameProperties& ) {}
public:
                                        SfxFrameProperties()
                                            : pFrame( 0 ),
                                              lMarginWidth( SIZE_NOT_SET ),
                                              lMarginHeight( SIZE_NOT_SET ),
                                              lSize( 1L ),
                                              lSetSize( 1L ),
                                              lFrameSpacing( SPACING_NOT_SET ),
                                              lInheritedFrameSpacing( SPACING_NOT_SET ),
                                              eScroll( ScrollingAuto ),
                                              eSizeSelector( SIZE_REL ),
                                              eSetSizeSelector( SIZE_REL ),
                                              bHasBorder( TRUE ),
                                              bHasBorderInherited( TRUE ),
                                              bBorderSet( TRUE ),
                                              bResizable( TRUE ),
                                              bSetResizable( TRUE ),
                                              bIsRootSet( FALSE ),
                                              bIsInColSet( FALSE ) {}

                                        ~SfxFrameProperties() { delete pFrame; }

    int             					operator ==( const SfxFrameProperties& ) const;
    SfxFrameProperties&				    operator =( const SfxFrameProperties &rProp );
};

class SfxFrameDescriptorItem : public SfxPoolItem
{
    SfxFrameProperties					aProperties;
public:
                                        TYPEINFO();

                                        SfxFrameDescriptorItem ( const USHORT nId = SID_FRAMEDESCRIPTOR )
                                            : SfxPoolItem( nId )
                                        {}

                                        SfxFrameDescriptorItem( const SfxFrameDescriptorItem& rCpy )
                                            : SfxPoolItem( rCpy )
                                        {
                                            aProperties = rCpy.aProperties;
                                        }

    virtual								~SfxFrameDescriptorItem();

    virtual int             			operator ==( const SfxPoolItem& ) const;
    SfxFrameDescriptorItem&				operator =( const SfxFrameDescriptorItem & );

    virtual SfxItemPresentation 		GetPresentation( SfxItemPresentation ePres,
                                            SfxMapUnit eCoreMetric,
                                            SfxMapUnit ePresMetric,
                                            UniString &rText, const ::IntlWrapper * = 0 ) const;

    virtual SfxPoolItem*    			Clone( SfxItemPool *pPool = 0 ) const;
    //virtual SfxPoolItem*    			Create(SvStream &, USHORT) const;
    //virtual SvStream&					Store(SvStream &, USHORT nItemVersion ) const;
    //virtual USHORT						GetVersion( USHORT nFileFormatVersion ) const;

    const SfxFrameProperties&			GetProperties() const
                                        { return aProperties; }
    void 								SetProperties( const SfxFrameProperties& rProp )
                                        { aProperties = rProp; }
};

}//end of namespace binfilter
#endif // #ifndef _SFX_FRMDESCRHXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
