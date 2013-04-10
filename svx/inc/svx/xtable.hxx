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

#ifndef _XTABLE_HXX
#define _XTABLE_HXX

// include ---------------------------------------------------------------

#include <svx/xpoly.hxx>
#include <svx/xdash.hxx>
#include <svx/xhatch.hxx>
#include <svx/xgrad.hxx>
#include <svx/xflasit.hxx>
#include <svx/xlnasit.hxx>
#include <tools/color.hxx>
#include <tools/string.hxx>
#include <tools/table.hxx>
#include <svx/svxdllapi.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/XPropertyEntry.hxx>

class Color;
class Bitmap;
class VirtualDevice;
class XOutdevItemPool;

// Standard-Vergleichsstring
extern sal_Unicode __FAR_DATA pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

// ------------------
// class XColorEntry
// ------------------

class XColorEntry : public XPropertyEntry
{
    Color   aColor;

public:
            XColorEntry(const Color& rColor, const String& rName) :
                XPropertyEntry(rName), aColor(rColor) {}
            XColorEntry(const XColorEntry& rOther) :
                XPropertyEntry(rOther), aColor(rOther.aColor) {}

    void    SetColor(const Color& rColor)   { aColor = rColor; }
    Color&  GetColor()                      { return aColor; }
};

// --------------------
// class XLineEndEntry
// --------------------

class XLineEndEntry : public XPropertyEntry
{
    basegfx::B2DPolyPolygon aB2DPolyPolygon;

public:
    XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const String& rName)
    :   XPropertyEntry(rName),
        aB2DPolyPolygon(rB2DPolyPolygon)
    {}

    XLineEndEntry(const XLineEndEntry& rOther)
    :   XPropertyEntry(rOther),
        aB2DPolyPolygon(rOther.aB2DPolyPolygon)
    {}

    void SetLineEnd(const basegfx::B2DPolyPolygon& rB2DPolyPolygon)
    {
        aB2DPolyPolygon = rB2DPolyPolygon;
    }

    basegfx::B2DPolyPolygon& GetLineEnd()
    {
        return aB2DPolyPolygon;
    }
};

// ------------------
// class XDashEntry
// ------------------

class XDashEntry : public XPropertyEntry
{
    XDash   aDash;

public:
            XDashEntry(const XDash& rDash, const String& rName) :
                XPropertyEntry(rName), aDash(rDash) {}
            XDashEntry(const XDashEntry& rOther) :
                XPropertyEntry(rOther), aDash(rOther.aDash) {}

    void    SetDash(const XDash& rDash)    { aDash = rDash; }
    XDash&  GetDash()                      { return aDash; }
};

// ------------------
// class XHatchEntry
// ------------------

class XHatchEntry : public XPropertyEntry
{
    XHatch  aHatch;

public:
            XHatchEntry(const XHatch& rHatch, const String& rName) :
                XPropertyEntry(rName), aHatch(rHatch) {}
            XHatchEntry(const XHatchEntry& rOther) :
                XPropertyEntry(rOther), aHatch(rOther.aHatch) {}

    void    SetHatch(const XHatch& rHatch)  { aHatch = rHatch; }
    XHatch& GetHatch()                      { return aHatch; }
};

// ---------------------
// class XGradientEntry
// ---------------------

class XGradientEntry : public XPropertyEntry
{
    XGradient  aGradient;

public:
                XGradientEntry(const XGradient& rGradient, const String& rName):
                    XPropertyEntry(rName), aGradient(rGradient) {}
                XGradientEntry(const XGradientEntry& rOther) :
                    XPropertyEntry(rOther), aGradient(rOther.aGradient) {}

    void        SetGradient(const XGradient& rGrad) { aGradient = rGrad; }
    XGradient&  GetGradient()                       { return aGradient; }
};

// ---------------------
// class XBitmapEntry
// ---------------------

class XBitmapEntry : public XPropertyEntry
{
private:
    GraphicObject   maGraphicObject;

public:
    XBitmapEntry(const GraphicObject& rGraphicObject, const String& rName)
    :   XPropertyEntry(rName),
        maGraphicObject(rGraphicObject)
    {
    }

    XBitmapEntry(const XBitmapEntry& rOther)
    :   XPropertyEntry(rOther),
        maGraphicObject(rOther.maGraphicObject)
    {
    }

    const GraphicObject& GetGraphicObject() const
    {
        return maGraphicObject;
    }

    void SetGraphicObject(const GraphicObject& rGraphicObject)
    {
        maGraphicObject = rGraphicObject;
    }
};

// --------------------
// class XPropertyList
// --------------------

class SVX_DLLPUBLIC XPropertyList
{
protected:
    String              maName; // nicht persistent !
    String              maPath;
    XOutdevItemPool*    mpXPool;

    List                maList;

    /// bitfield
    bool                mbListDirty : 1;

    XPropertyList( const String& rPath, XOutdevItemPool* pXPool = 0 );
    void                Clear();
    virtual Bitmap      CreateBitmapForUI( long nIndex ) = 0;

public:
    virtual             ~XPropertyList();

    long                Count() const;

    void                Insert( XPropertyEntry* pEntry, long nIndex = LIST_APPEND );
    XPropertyEntry*     Replace( XPropertyEntry* pEntry, long nIndex );
    XPropertyEntry*     Remove( long nIndex, sal_uInt16 nDummy );
    XPropertyEntry*     Get( long nIndex, sal_uInt16 nDummy ) const;

    long                Get(const String& rName);
    Bitmap              GetUiBitmap( long nIndex ) const;

    const String&       GetName() const { return maName; }
    void                SetName( const String& rString );
    const String&       GetPath() const { return maPath; }
    void                SetPath( const String& rString ) { maPath = rString; }
    bool                IsDirty() const { return mbListDirty; }
    void                SetDirty( bool bDirty = true ) { mbListDirty = bDirty; }

    virtual sal_Bool        Load() = 0;
    virtual sal_Bool        Save() = 0;
    virtual sal_Bool        Create() = 0;

    sal_uInt32 getUiBitmapWidth() const;
    sal_uInt32 getUiBitmapHeight() const;
    sal_uInt32 getUiBitmapLineWidth() const;
};

// -------------------
// class XColorList
// -------------------

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
protected:
    virtual Bitmap  CreateBitmapForUI( long nIndex );

public:
                    XColorList( const String& rPath, XOutdevItemPool* pXPool = 0 );
    virtual         ~XColorList();

    using XPropertyList::Replace;
    XColorEntry*    Replace(XColorEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XColorEntry*    Remove(long nIndex);
    using XPropertyList::Get;
    XColorEntry*    GetColor(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();

    static XColorList*  GetStdColorList();
};

// -------------------
// class XLineEndList
// -------------------
class impXLineEndList;

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
private:
    impXLineEndList*    mpData;

    void impCreate();
    void impDestroy();

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    XLineEndList(const String& rPath, XOutdevItemPool* pXPool = 0);
    virtual ~XLineEndList();

    using XPropertyList::Replace;
    XLineEndEntry* Replace(XLineEndEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
};

// -------------------
// class XDashList
// -------------------
class impXDashList;

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    impXDashList*       mpData;
    Bitmap              maBitmapSolidLine;
    String              maStringSolidLine;
    String              maStringNoLine;

    void impCreate();
    void impDestroy();

protected:
    Bitmap ImpCreateBitmapForXDash(const XDash* pDash);
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    XDashList(const String& rPath, XOutdevItemPool* pXPool = 0);
    virtual ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XDashEntry* GetDash(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    Bitmap GetBitmapForUISolidLine() const;

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    String GetStringForUiSolidLine() const;
    String GetStringForUiNoLine() const;
};

// -------------------
// class XHatchList
// -------------------
class impXHatchList;

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
private:
    impXHatchList*      mpData;

    void impCreate();
    void impDestroy();

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    XHatchList(const String& rPath, XOutdevItemPool* pXPool = 0);
    ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XHatchEntry* GetHatch(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
};

// -------------------
// class XGradientList
// -------------------
class impXGradientList;

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
private:
    impXGradientList*   mpData;

    void impCreate();
    void impDestroy();

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    XGradientList(const String& rPath, XOutdevItemPool* pXPool = 0);
    virtual ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    using XPropertyList::Get;
    XGradientEntry* GetGradient(long nIndex) const;

    virtual sal_Bool Load();
    virtual sal_Bool Save();
    virtual sal_Bool Create();
};

// -------------------
// class XBitmapList
// -------------------

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
protected:
    virtual Bitmap CreateBitmapForUI( long nIndex );

public:
                    XBitmapList( const String& rPath, XOutdevItemPool* pXPool = 0);
    virtual         ~XBitmapList();

    using XPropertyList::Replace;
    XBitmapEntry*   Replace(XBitmapEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XBitmapEntry*   Remove(long nIndex);
    using XPropertyList::Get;
    XBitmapEntry*   GetBitmap(long nIndex) const;

    virtual sal_Bool    Load();
    virtual sal_Bool    Save();
    virtual sal_Bool    Create();
};

#endif // _XTABLE_HXX

// eof
