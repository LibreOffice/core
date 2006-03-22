/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xldumper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-03-22 12:08:50 $
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

#ifndef SC_XLDUMPER_HXX
#define SC_XLDUMPER_HXX

#ifndef SC_FDUMPER_HXX
#include "fdumper.hxx"
#endif

#if SCF_INCL_DUMPER

class SfxMedium;
class SfxObjectShell;
class XclImpStream;

namespace scf {
namespace xls {
namespace dump {

class CoreRootData;
class FilterRootData;

typedef ScfRef< CoreRootData >      CoreRootDataRef;
typedef ScfRef< FilterRootData >    FilterRootDataRef;

// ============================================================================
// ============================================================================

class Config : public ::scf::dump::Config
{
public:
    explicit            Config();
};

// ============================================================================
// ============================================================================

class RecordStreamObject : public ::scf::dump::InputObject
{
public:
    explicit            RecordStreamObject( const ::scf::dump::BaseObject& rParent,
                            CoreRootDataRef xCoreRoot, SvStream& rStrm );
    explicit            RecordStreamObject( const ::scf::dump::OleStorageObject& rParentStrg,
                            CoreRootDataRef xCoreRoot, const String& rStrmName );
    virtual             ~RecordStreamObject();

protected:
    explicit            RecordStreamObject();
    void                Construct( const ::scf::dump::BaseObject& rParent,
                            CoreRootDataRef xCoreRoot, SvStream& rStrm,
                            const String& rProgressName );
    void                Construct( const ::scf::dump::OleStorageObject& rParentStrg,
                            CoreRootDataRef xCoreRoot, const String& rStrmName );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpBody();
    virtual void        ImplDumpFooter();

    virtual void        PrepareRecord( XclImpStream& rStrm, bool bMergeContRec );
    virtual void        DumpRecord( XclImpStream& rStrm );

private:
    typedef ScfRef< XclImpStream >                  XclImpStreamRef;
    typedef ScfRef< ::scf::dump::OleStreamObject >  OleStreamObjectRef;

    CoreRootDataRef     mxCoreRoot;
    FilterRootDataRef   mxFilterRoot;
    XclImpStreamRef     mxStrm;
    OleStreamObjectRef  mxOleStrm;
    String              maProgressName;
};

// ============================================================================

class WorkbookStreamObject : public RecordStreamObject
{
public:
    explicit            WorkbookStreamObject( const ::scf::dump::BaseObject& rParent,
                            CoreRootDataRef xCoreRoot, SvStream& rStrm );
    explicit            WorkbookStreamObject( const ::scf::dump::OleStorageObject& rParentStrg,
                            CoreRootDataRef xCoreRoot, const String& rStrmName );

protected:
    virtual void        PrepareRecord( XclImpStream& rStrm, bool bMergeContRec );
    virtual void        DumpRecord( XclImpStream& rStrm );

private:
    void                InitMembers();

private:
    typedef ::scf::dump::NameListRef NameListRef;
    NameListRef         mxBofBiffTypes;
    NameListRef         mxBofSheetTypes;
    NameListRef         mxBofHistFlags;
};

// ============================================================================
// ============================================================================

class RootStorageObject : public ::scf::dump::OleStorageObject
{
public:
    explicit            RootStorageObject( const ::scf::dump::BaseObject& rParent,
                            CoreRootDataRef xCoreRoot );
    virtual             ~RootStorageObject();

protected:
    explicit            RootStorageObject();
    void                Construct( const ::scf::dump::BaseObject& rParent,
                            CoreRootDataRef xCoreRoot );

    virtual void        ImplDumpBody();

private:
    CoreRootDataRef     mxCoreRoot;
};

// ============================================================================

class Dumper : public ::scf::dump::BaseObject
{
public:
    explicit            Dumper( SfxMedium& rMedium, SfxObjectShell* pDocShell );
    virtual             ~Dumper();

protected:
    virtual void        ImplDumpBody();

private:
    CoreRootDataRef     mxCoreRoot;
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace xls
} // namespace scf

#endif
#endif

