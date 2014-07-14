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
#include <svtools/grfmgr.hxx>
#include <svx/XPropertyEntry.hxx>
#include <vcl/bitmap.hxx>

// Standard-Vergleichsstring
extern sal_Unicode __FAR_DATA pszStandard[]; // "standard"

// Funktion zum Konvertieren in echte RGB-Farben, da mit
// enum COL_NAME nicht verglichen werden kann.
SVX_DLLPUBLIC Color RGB_Color( ColorData nColorName );

//////////////////////////////////////////////////////////////////////////////
// class XColorEntry

class SVX_DLLPUBLIC XColorEntry : public XPropertyEntry
{
private:
    Color   aColor;

public:
    XColorEntry(const Color& rColor, const String& rName);
    XColorEntry(const XColorEntry& rOther);

    const Color& GetColor() const
    {
        return aColor;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XLineEndEntry

class SVX_DLLPUBLIC XLineEndEntry : public XPropertyEntry
{
private:
    basegfx::B2DPolyPolygon aB2DPolyPolygon;

public:
    XLineEndEntry(const basegfx::B2DPolyPolygon& rB2DPolyPolygon, const String& rName);
    XLineEndEntry(const XLineEndEntry& rOther);

    const basegfx::B2DPolyPolygon& GetLineEnd() const
    {
        return aB2DPolyPolygon;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XDashEntry

class SVX_DLLPUBLIC XDashEntry : public XPropertyEntry
{
private:
    XDash   aDash;

public:
    XDashEntry(const XDash& rDash, const String& rName);
    XDashEntry(const XDashEntry& rOther);

    const XDash& GetDash() const
    {
        return aDash;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XHatchEntry

class SVX_DLLPUBLIC XHatchEntry : public XPropertyEntry
{
private:
    XHatch  aHatch;

public:
    XHatchEntry(const XHatch& rHatch, const String& rName);
    XHatchEntry(const XHatchEntry& rOther);

    const XHatch& GetHatch() const
    {
        return aHatch;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XGradientEntry

class SVX_DLLPUBLIC XGradientEntry : public XPropertyEntry
{
private:
    XGradient  aGradient;

public:
    XGradientEntry(const XGradient& rGradient, const String& rName);
    XGradientEntry(const XGradientEntry& rOther);

    const XGradient& GetGradient() const
    {
        return aGradient;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XBitmapEntry

class SVX_DLLPUBLIC XBitmapEntry : public XPropertyEntry
{
private:
    GraphicObject   maGraphicObject;

public:
    XBitmapEntry(const GraphicObject& rGraphicObject, const String& rName);
    XBitmapEntry(const XBitmapEntry& rOther);

    const GraphicObject& GetGraphicObject() const
    {
        return maGraphicObject;
    }
};

//////////////////////////////////////////////////////////////////////////////
// class XPropertyList

class SVX_DLLPUBLIC XPropertyList
{
protected:
    String              maName; // nicht persistent !
    String              maPath;

    ::std::vector< XPropertyEntry* >    maContent;

    /// bitfield
    bool                mbListDirty : 1;

    XPropertyList(const String& rPath);
    void Clear();
    virtual Bitmap CreateBitmapForUI(long nIndex) = 0;

public:
    virtual ~XPropertyList();

    long Count() const;

    void Insert(XPropertyEntry* pEntry, long nIndex = LIST_APPEND);
    XPropertyEntry* Replace(XPropertyEntry* pEntry, long nIndex);
    XPropertyEntry* Remove(long nIndex);
    XPropertyEntry* Get(long nIndex) const;

    long GetIndex(const String& rName) const;
    Bitmap GetUiBitmap(long nIndex) const;

    const String& GetName() const { return maName; }
    void SetName( const String& rString );

    const String& GetPath() const { return maPath; }
    void SetPath( const String& rString ) { maPath = rString; }

    bool IsDirty() const { return mbListDirty; }
    void SetDirty( bool bDirty = true ) { mbListDirty = bDirty; }

    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool Create() = 0;
};

//////////////////////////////////////////////////////////////////////////////
// predefines for XList classes and the SharedPtr typedefs for these to have
// them in a central place for better overview

class XColorList;
class XLineEndList;
class XDashList;
class XHatchList;
class XGradientList;
class XBitmapList;

typedef ::boost::shared_ptr< XColorList > XColorListSharedPtr;
typedef ::boost::shared_ptr< XLineEndList > XLineEndListSharedPtr;
typedef ::boost::shared_ptr< XDashList > XDashListSharedPtr;
typedef ::boost::shared_ptr< XHatchList > XHatchListSharedPtr;
typedef ::boost::shared_ptr< XGradientList > XGradientListSharedPtr;
typedef ::boost::shared_ptr< XBitmapList > XBitmapListSharedPtr;

//////////////////////////////////////////////////////////////////////////////
// XPropertyListFactory to limit XListSharedPtr creation and thus XPropertyList creation
// to this factory, so no one can instantiate a non-shared instace of XPropertyList
// or it's derivates

class SVX_DLLPUBLIC XPropertyListFactory
{
public:
    static XColorListSharedPtr CreateSharedXColorList( const String& rPath );
    static XLineEndListSharedPtr CreateSharedXLineEndList( const String& rPath );
    static XDashListSharedPtr CreateSharedXDashList( const String& rPath );
    static XHatchListSharedPtr CreateSharedXHatchList( const String& rPath );
    static XGradientListSharedPtr CreateSharedXGradientList( const String& rPath );
    static XBitmapListSharedPtr CreateSharedXBitmapList( const String& rPath );
};

//////////////////////////////////////////////////////////////////////////////
// class XColorList

class SVX_DLLPUBLIC XColorList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XColorList(const String& rPath);

protected:
    virtual Bitmap  CreateBitmapForUI(long nIndex);

public:
    virtual ~XColorList();

    using XPropertyList::Replace;
    XColorEntry* Replace(XColorEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XColorEntry* Remove(long nIndex);
    XColorEntry* GetColor(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();

    static XColorListSharedPtr GetStdColorList();
};

//////////////////////////////////////////////////////////////////////////////
// class XLineEndList

class SVX_DLLPUBLIC XLineEndList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XLineEndList(const String& rPath);

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    virtual ~XLineEndList();

    using XPropertyList::Replace;
    XLineEndEntry* Replace(XLineEndEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XLineEndEntry* Remove(long nIndex);
    XLineEndEntry* GetLineEnd(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();
};

//////////////////////////////////////////////////////////////////////////////
// class XDashList

class SVX_DLLPUBLIC XDashList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XDashList(const String& rPath);

    Bitmap              maBitmapSolidLine;
    String              maStringSolidLine;
    String              maStringNoLine;

protected:
    Bitmap ImpCreateBitmapForXDash(const XDash* pDash);
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    virtual ~XDashList();

    using XPropertyList::Replace;
    XDashEntry* Replace(XDashEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XDashEntry* Remove(long nIndex);
    XDashEntry* GetDash(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();

    // Special call to get a bitmap for the solid line representation. It
    // creates a bitmap fitting in size and style to the ones you get by
    // using GetUiBitmap for existing entries.
    Bitmap GetBitmapForUISolidLine() const;

    // Special calls to get the translated strings for the UI entry for no
    // line style (XLINE_NONE) and solid line style (XLINE_SOLID) for dialogs
    String GetStringForUiSolidLine() const;
    String GetStringForUiNoLine() const;
};

//////////////////////////////////////////////////////////////////////////////
// class XHatchList

class SVX_DLLPUBLIC XHatchList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XHatchList(const String& rPath);

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    ~XHatchList();

    using XPropertyList::Replace;
    XHatchEntry* Replace(XHatchEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XHatchEntry* Remove(long nIndex);
    XHatchEntry* GetHatch(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();
};

//////////////////////////////////////////////////////////////////////////////
// class XGradientList

class SVX_DLLPUBLIC XGradientList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XGradientList(const String& rPath);

protected:
    virtual Bitmap CreateBitmapForUI(long nIndex);

public:
    virtual ~XGradientList();

    using XPropertyList::Replace;
    XGradientEntry* Replace(XGradientEntry* pEntry, long nIndex);
    using XPropertyList::Remove;
    XGradientEntry* Remove(long nIndex);
    XGradientEntry* GetGradient(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();
};

//////////////////////////////////////////////////////////////////////////////
// class XBitmapList

class SVX_DLLPUBLIC XBitmapList : public XPropertyList
{
private:
    friend class XPropertyListFactory;
    XBitmapList(const String& rPath);

protected:
    virtual Bitmap CreateBitmapForUI( long nIndex );

public:
    virtual ~XBitmapList();

    using XPropertyList::Replace;
    XBitmapEntry* Replace(XBitmapEntry* pEntry, long nIndex );
    using XPropertyList::Remove;
    XBitmapEntry* Remove(long nIndex);
    XBitmapEntry* GetBitmap(long nIndex) const;

    virtual bool Load();
    virtual bool Save();
    virtual bool Create();
};

//////////////////////////////////////////////////////////////////////////////

#endif // _XTABLE_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
