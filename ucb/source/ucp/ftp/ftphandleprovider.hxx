/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <rtl/ustring.hxx>
#include "curl.hxx"

namespace ftp {

    class FTPHandleProvider {
    public:

        virtual CURL* handle() = 0;


        /** host is in the form host:port.
         */

        virtual bool forHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             rtl::OUString& password,
                             rtl::OUString& account) = 0;

        virtual bool setHost(const rtl::OUString& host,
                             const rtl::OUString& port,
                             const rtl::OUString& username,
                             const rtl::OUString& password,
                             const rtl::OUString& account) = 0;
    };


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
