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

#ifndef INCLUDED_OOX_DUMP_XLSBDUMPER_HXX
#define INCLUDED_OOX_DUMP_XLSBDUMPER_HXX

#include <oox/dump/dumperbase.hxx>

#if OOX_INCLUDE_DUMPER

namespace oox { namespace xls {
    class FontPortionModelList;
    class PhoneticPortionModelList;
    struct FunctionInfo;
    class FunctionProvider;
} }

namespace oox {
namespace dump {
namespace xlsb {



class RecordObjectBase : public SequenceRecordObjectBase
{
protected:
    explicit            RecordObjectBase();
    virtual             ~RecordObjectBase();

    using               SequenceRecordObjectBase::construct;

    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize ) override;

private:
    typedef std::shared_ptr< SequenceInputStream > SequenceInputStreamRef;

    SequenceInputStreamRef mxBiffStrm;
    NameListRef         mxErrCodes;
};



class RootStorageObject : public StorageObjectBase
{
public:
    explicit            RootStorageObject( const DumperBase& rParent );

protected:
    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName ) override;
};



class Dumper : public DumperBase
{
public:
    explicit            Dumper( const ::oox::core::FilterBase& rFilter );

    explicit            Dumper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxInStrm,
                            const OUString& rSysFileName );

protected:
    virtual void        implDump() override;
};



} // namespace xlsb
} // namespace dump
} // namespace oox

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
