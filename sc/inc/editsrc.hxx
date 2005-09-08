/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editsrc.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:38:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_EDITSRC_HXX
#define SC_EDITSRC_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include <svx/unoedsrc.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#include <memory>

class ScEditEngineDefaulter;
class SvxEditEngineForwarder;

class ScDocShell;
class ScHeaderFooterContentObj;
class ScCellTextData;
class ScHeaderFooterTextData;
class ScAccessibleTextData;
class SdrObject;


class ScHeaderFooterChangedHint : public SfxHint
{
    USHORT nPart;

public:
                    TYPEINFO();
                    ScHeaderFooterChangedHint(USHORT nP);
                    ~ScHeaderFooterChangedHint();

    USHORT          GetPart() const         { return nPart; }
};


//  all ScSharedHeaderFooterEditSource objects for a single text share the same data

class ScSharedHeaderFooterEditSource : public SvxEditSource
{
private:
    ScHeaderFooterTextData*     pTextData;

protected:
    ScHeaderFooterTextData*     GetTextData() const { return pTextData; }   // for ScHeaderFooterEditSource

public:
                                ScSharedHeaderFooterEditSource( ScHeaderFooterTextData* pData );
    virtual                     ~ScSharedHeaderFooterEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*      GetEditEngine();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();

    virtual void                UpdateData();

};

//  ScHeaderFooterEditSource with local copy of ScHeaderFooterTextData is used by field objects

class ScHeaderFooterEditSource : public ScSharedHeaderFooterEditSource
{
public:
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj* pContent, USHORT nP );
                                ScHeaderFooterEditSource( ScHeaderFooterContentObj& rContent, USHORT nP );
    virtual                     ~ScHeaderFooterEditSource();

    virtual SvxEditSource*      Clone() const;
};


//  Data (incl. EditEngine) for cell EditSource is now shared in ScCellTextData

class ScSharedCellEditSource : public SvxEditSource
{
private:
    ScCellTextData*         pCellTextData;

protected:
    ScCellTextData*         GetCellTextData() const { return pCellTextData; }   // for ScCellEditSource

public:
                                ScSharedCellEditSource( ScCellTextData* pData );
    virtual                     ~ScSharedCellEditSource();

    //  GetEditEngine is needed because the forwarder doesn't have field functions
    ScEditEngineDefaulter*      GetEditEngine();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();

    virtual void                UpdateData();

    void                        SetDoUpdateData(sal_Bool bValue);
    sal_Bool                    IsDirty() const;
};

//  ScCellEditSource with local copy of ScCellTextData is used by ScCellFieldsObj, ScCellFieldObj

class ScCellEditSource : public ScSharedCellEditSource
{
public:
                                ScCellEditSource( ScDocShell* pDocSh, const ScAddress& rP );
    virtual                     ~ScCellEditSource();

    virtual SvxEditSource*      Clone() const;
};


class ScAnnotationEditSource : public SvxEditSource, public SfxListener
{
private:
    ScDocShell*             pDocShell;
    ScAddress               aCellPos;
    ScEditEngineDefaulter*  pEditEngine;
    SvxEditEngineForwarder* pForwarder;
    BOOL                    bDataValid;

    SdrObject*                  GetCaptionObj();
public:
                                ScAnnotationEditSource(ScDocShell* pDocSh, const ScAddress& rP);
    virtual                     ~ScAnnotationEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

    virtual void                Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};


//  EditSource with a shared forwarder for all children of one text object

class ScSimpleEditSource : public SvxEditSource
{
private:
    SvxTextForwarder*   pForwarder;

public:
                        ScSimpleEditSource( SvxTextForwarder* pForw );
    virtual             ~ScSimpleEditSource();

    virtual SvxEditSource*      Clone() const ;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual void                UpdateData();

};

class ScAccessibilityEditSource : public SvxEditSource
{
private:
    ::std::auto_ptr < ScAccessibleTextData > mpAccessibleTextData;

public:
                        ScAccessibilityEditSource( ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData );
    virtual             ~ScAccessibilityEditSource();

    virtual SvxEditSource*      Clone() const;
    virtual SvxTextForwarder*   GetTextForwarder();
    virtual SvxViewForwarder*   GetViewForwarder();
    virtual SvxEditViewForwarder*   GetEditViewForwarder( sal_Bool bCreate = sal_False );
    virtual void                UpdateData();
    virtual SfxBroadcaster&     GetBroadcaster() const;

    void                        SetDoUpdateData(sal_Bool bValue);
    sal_Bool                    IsDirty() const;
};

#endif

