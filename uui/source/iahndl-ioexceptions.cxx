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

#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/task/XInteractionRequest.hpp"
#include "com/sun/star/ucb/InteractiveAugmentedIOException.hpp"

#include "ids.hrc"

#include "iahndl.hxx"

using namespace com::sun::star;

namespace {

bool
getStringRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                         rtl::OUString const & rKey,
                         rtl::OUString * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            rtl::OUString aValue;
            if (aProperty.Value >>= aValue)
            {
                if (pValue)
                    *pValue = aValue;
                return true;
            }
        }
    }
    return false;
}

bool
getBoolRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                       rtl::OUString const & rKey,
                       bool * pValue)
    SAL_THROW(())
{
    for (sal_Int32 i = 0; i < rArguments.getLength(); ++i)
    {
        beans::PropertyValue aProperty;
        if ((rArguments[i] >>= aProperty) && aProperty.Name == rKey)
        {
            sal_Bool bValue = sal_Bool();
            if (aProperty.Value >>= bValue)
            {
                if (pValue)
                    *pValue = bValue;
                return true;
            }
        }
    }
    return false;
}

bool
getResourceNameRequestArgument(uno::Sequence< uno::Any > const & rArguments,
                               rtl::OUString * pValue)
    SAL_THROW(())
{
    if (!getStringRequestArgument(rArguments,
                                  rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                      "Uri")),
                                  pValue))
        return false;
    // Use the resource name only for file URLs, to avoid confusion:
    //TODO! work with ucp locality concept instead of hardcoded "file"?
    if (pValue
        && pValue->matchIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM(
                                                  "file:")))
        getStringRequestArgument(rArguments,
                                 rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                                   "ResourceName")),
                                 pValue);
    return true;
}

} // namespace

bool
UUIInteractionHelper::handleInteractiveIOException(
        uno::Reference< task::XInteractionRequest > const & rRequest,
        bool bObtainErrorStringOnly,
        bool & bHasErrorString,
        rtl::OUString & rErrorString)
    SAL_THROW((uno::RuntimeException))
{
    uno::Any aAnyRequest(rRequest->getRequest());
    bHasErrorString = false;

    ucb::InteractiveIOException aIoException;
    if (aAnyRequest >>= aIoException)
    {
        uno::Sequence< uno::Any > aRequestArguments;
        ucb::InteractiveAugmentedIOException aAugmentedIoException;
        if (aAnyRequest >>= aAugmentedIoException)
            aRequestArguments = aAugmentedIoException.Arguments;

        ErrCode nErrorCode;
        std::vector< rtl::OUString > aArguments;
        static ErrCode const
            aErrorCode[ucb::IOErrorCode_WRONG_VERSION + 1][2]
            = { { ERRCODE_IO_ABORT, ERRCODE_UUI_IO_ABORT }, // ABORT
                { ERRCODE_IO_ACCESSDENIED, ERRCODE_UUI_IO_ACCESSDENIED },
                // ACCESS_DENIED
                { ERRCODE_IO_ALREADYEXISTS,
                  ERRCODE_UUI_IO_ALREADYEXISTS }, // ALREADY_EXISTING
                { ERRCODE_IO_BADCRC, ERRCODE_UUI_IO_BADCRC }, // BAD_CRC
                { ERRCODE_IO_CANTCREATE, ERRCODE_UUI_IO_CANTCREATE },
                // CANT_CREATE
                { ERRCODE_IO_CANTREAD, ERRCODE_UUI_IO_CANTREAD },
                // CANT_READ
                { ERRCODE_IO_CANTSEEK, ERRCODE_UUI_IO_CANTSEEK },
                // CANT_SEEK
                { ERRCODE_IO_CANTTELL, ERRCODE_UUI_IO_CANTTELL },
                // CANT_TELL
                { ERRCODE_IO_CANTWRITE, ERRCODE_UUI_IO_CANTWRITE },
                // CANT_WRITE
                { ERRCODE_IO_CURRENTDIR, ERRCODE_UUI_IO_CURRENTDIR },
                // CURRENT_DIRECTORY
                { ERRCODE_IO_DEVICENOTREADY, ERRCODE_UUI_IO_NOTREADY },
                // DEVICE_NOT_READY
                { ERRCODE_IO_NOTSAMEDEVICE,
                  ERRCODE_UUI_IO_NOTSAMEDEVICE }, // DIFFERENT_DEVICES
                { ERRCODE_IO_GENERAL, ERRCODE_UUI_IO_GENERAL }, // GENERAL
                { ERRCODE_IO_INVALIDACCESS,
                  ERRCODE_UUI_IO_INVALIDACCESS }, // INVALID_ACCESS
                { ERRCODE_IO_INVALIDCHAR, ERRCODE_UUI_IO_INVALIDCHAR },
                // INVALID_CHARACTER
                { ERRCODE_IO_INVALIDDEVICE,
                  ERRCODE_UUI_IO_INVALIDDEVICE }, // INVALID_DEVICE
                { ERRCODE_IO_INVALIDLENGTH,
                  ERRCODE_UUI_IO_INVALIDLENGTH }, // INVALID_LENGTH
                { ERRCODE_IO_INVALIDPARAMETER,
                  ERRCODE_UUI_IO_INVALIDPARAMETER }, // INVALID_PARAMETER
                { ERRCODE_IO_ISWILDCARD, ERRCODE_UUI_IO_ISWILDCARD },
                // IS_WILDCARD
                { ERRCODE_IO_LOCKVIOLATION,
                  ERRCODE_UUI_IO_LOCKVIOLATION }, // LOCKING_VIOLATION
                { ERRCODE_IO_MISPLACEDCHAR,
                  ERRCODE_UUI_IO_MISPLACEDCHAR }, // MISPLACED_CHARACTER
                { ERRCODE_IO_NAMETOOLONG, ERRCODE_UUI_IO_NAMETOOLONG },
                // NAME_TOO_LONG
                { ERRCODE_IO_NOTEXISTS, ERRCODE_UUI_IO_NOTEXISTS },
                // NOT_EXISTING
                { ERRCODE_IO_NOTEXISTSPATH,
                  ERRCODE_UUI_IO_NOTEXISTSPATH }, // NOT_EXISTING_PATH
                { ERRCODE_IO_NOTSUPPORTED, ERRCODE_UUI_IO_NOTSUPPORTED },
                // NOT_SUPPORTED
                { ERRCODE_IO_NOTADIRECTORY,
                  ERRCODE_UUI_IO_NOTADIRECTORY }, // NO_DIRECTORY
                { ERRCODE_IO_NOTAFILE, ERRCODE_UUI_IO_NOTAFILE },
                // NO_FILE
                { ERRCODE_IO_OUTOFSPACE, ERRCODE_UUI_IO_OUTOFSPACE },
                // OUT_OF_DISK_SPACE
                { ERRCODE_IO_TOOMANYOPENFILES,
                  ERRCODE_UUI_IO_TOOMANYOPENFILES },
                // OUT_OF_FILE_HANDLES
                { ERRCODE_IO_OUTOFMEMORY, ERRCODE_UUI_IO_OUTOFMEMORY },
                // OUT_OF_MEMORY
                { ERRCODE_IO_PENDING, ERRCODE_UUI_IO_PENDING }, // PENDING
                { ERRCODE_IO_RECURSIVE, ERRCODE_UUI_IO_RECURSIVE },
                // RECURSIVE
                { ERRCODE_IO_UNKNOWN, ERRCODE_UUI_IO_UNKNOWN }, // UNKNOWN
                { ERRCODE_IO_WRITEPROTECTED,
                  ERRCODE_UUI_IO_WRITEPROTECTED }, // WRITE_PROTECTED
                { ERRCODE_IO_WRONGFORMAT, ERRCODE_UUI_IO_WRONGFORMAT },
                // WRONG_FORMAT
                { ERRCODE_IO_WRONGVERSION,
                  ERRCODE_UUI_IO_WRONGVERSION } }; // WRONG_VERSION
        switch (aIoException.Code)
        {
        case ucb::IOErrorCode_CANT_CREATE:
            {
                rtl::OUString aArgFolder;
                if (getStringRequestArgument(
                        aRequestArguments,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "Folder")),
                        &aArgFolder))
                {
                    rtl::OUString aArgUri;
                    if (getResourceNameRequestArgument(aRequestArguments,
                                                       &aArgUri))
                    {
                        nErrorCode = ERRCODE_UUI_IO_CANTCREATE;
                        aArguments.reserve(2);
                        aArguments.push_back(aArgUri);
                        aArguments.push_back(aArgFolder);
                    }
                    else
                    {
                        nErrorCode = ERRCODE_UUI_IO_CANTCREATE_NONAME;
                        aArguments.push_back(aArgFolder);
                    }
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        case ucb::IOErrorCode_DEVICE_NOT_READY:
            {
                rtl::OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                                                   &aArgUri))
                {
                    rtl::OUString aResourceType;
                    getStringRequestArgument(
                        aRequestArguments,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "ResourceType")),
                        &aResourceType);
                    bool bRemovable = false;
                    getBoolRequestArgument(aRequestArguments,
                                           rtl::OUString(
                                               RTL_CONSTASCII_USTRINGPARAM(
                                                   "Removable")),
                                           &bRemovable);
                    nErrorCode
                        = aResourceType.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM("volume"))
                        ? (bRemovable
                           ? ERRCODE_UUI_IO_NOTREADY_VOLUME_REMOVABLE
                           : ERRCODE_UUI_IO_NOTREADY_VOLUME)
                        : (bRemovable
                           ? ERRCODE_UUI_IO_NOTREADY_REMOVABLE
                           : ERRCODE_UUI_IO_NOTREADY);
                    aArguments.push_back(aArgUri);
                }
                else
            nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        case ucb::IOErrorCode_DIFFERENT_DEVICES:
            {
                rtl::OUString aArgVolume;
                rtl::OUString aArgOtherVolume;
                if (getStringRequestArgument(
                        aRequestArguments,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "Volume")),
                        &aArgVolume)
                    && getStringRequestArgument(
                        aRequestArguments,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "OtherVolume")),
                        &aArgOtherVolume))
                {
                    nErrorCode = aErrorCode[aIoException.Code][1];
                    aArguments.reserve(2);
                    aArguments.push_back(aArgVolume);
                    aArguments.push_back(aArgOtherVolume);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
        }

        case ucb::IOErrorCode_NOT_EXISTING:
            {
                rtl::OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                           &aArgUri))
                {
                    rtl::OUString aResourceType;
                    getStringRequestArgument(
                        aRequestArguments,
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                                          "ResourceType")),
                        &aResourceType);
                    nErrorCode
                        = aResourceType.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM("volume"))
                        ? ERRCODE_UUI_IO_NOTEXISTS_VOLUME
                        : (aResourceType.equalsAsciiL(
                               RTL_CONSTASCII_STRINGPARAM("folder"))
                           ? ERRCODE_UUI_IO_NOTEXISTS_FOLDER
                           : ERRCODE_UUI_IO_NOTEXISTS);
                    aArguments.push_back(aArgUri);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }

        default:
            {
                rtl::OUString aArgUri;
                if (getResourceNameRequestArgument(aRequestArguments,
                                                   &aArgUri))
                {
                    nErrorCode = aErrorCode[aIoException.Code][1];
                    aArguments.push_back(aArgUri);
                }
                else
                    nErrorCode = aErrorCode[aIoException.Code][0];
                break;
            }
        }

        handleErrorHandlerRequest(aIoException.Classification,
                                  nErrorCode,
                                  aArguments,
                                  rRequest->getContinuations(),
                                  bObtainErrorStringOnly,
                                  bHasErrorString,
                                  rErrorString);
        return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
