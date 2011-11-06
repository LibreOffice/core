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



#ifndef OOX_DUMP_DFFDUMPER_HXX
#define OOX_DUMP_DFFDUMPER_HXX

#include "oox/dump/dumperbase.hxx"

#if OOX_INCLUDE_DUMPER

namespace oox {
namespace dump {

// ============================================================================

class DffStreamObject : public SequenceRecordObjectBase
{
public:
    inline sal_uInt16   getVer() const { return mnInstVer & 0x000F; }
    inline sal_uInt16   getInst() const { return (mnInstVer & 0xFFF0) >> 4; }
    inline bool         isContainer() const { return getVer() == 15; }

protected:
    inline explicit     DffStreamObject() {}

    using               SequenceRecordObjectBase::construct;
    void                construct( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );
    void                construct( const OutputObjectBase& rParent, const BinaryInputStreamRef& rxStrm );

    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );
    virtual void        implWriteExtHeader();
    virtual void        implDumpRecordBody();
    virtual void        implDumpClientAnchor();

private:
    void                constructDffObj();

    sal_uInt32          dumpDffSimpleColor( const String& rName );
    sal_uInt32          dumpDffColor( const String& rName );

    void                dumpDffOpt();
    sal_uInt16          dumpDffOptPropHeader();

private:
    ItemFormatMap       maSimpleProps;
    ItemFormatMap       maComplexProps;
    sal_uInt16          mnInstVer;
    sal_Int32           mnRealSize;
};

// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif
