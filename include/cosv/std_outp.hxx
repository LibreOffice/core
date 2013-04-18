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

#ifndef CSV_STD_OUTP_HXX
#define CSV_STD_OUTP_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <cosv/csv_ostream.hxx>
    // PARAMETERS




namespace csv
{

#ifdef CSV_NO_IOSTREAMS
class redirect_out : public ostream
{
  public:
    virtual             ~redirect_out() {}

    static void         set_(
                            redirect_out &      o_rStdOut,
                            redirect_out &      o_rStdErr )
                                                { pStdOut_ = &o_rStdOut;
                                                  pStdErr_ = &o_rStdErr; }

    static redirect_out &
                        std_()                  { return *pStdOut_; }
    static redirect_out &
                        err_()                  { return *pStdErr_; }
    static bool         useme_()                { return pStdOut_ != 0; }

  private:
    // DATA
    static redirect_out *
                        pStdOut_;
    static redirect_out *
                        pStdErr_;
};
#endif // defined(CSV_NO_IOSTREAMS)


inline ostream &
Cout()
{

#ifndef CSV_NO_IOSTREAMS
//    return redirect_out::useme_()
//            ?   (ostream&)( redirect_out::std_() )
//            :   (ostream&)( std::cout );
    return (ostream&)( std::cout );
#else
    csv_assert( redirect_out::useme_() );
    return redirect_out::std_();
#endif
}

inline ostream &
Cerr()
{
#ifndef CSV_NO_IOSTREAMS
//    return redirect_out::useme_()
//            ?   (ostream&)( redirect_out::err_() )
//            :   (ostream&)( std::cerr );
    return (ostream&)( std::cerr );
#else
    csv_assert( redirect_out::useme_() );
    return redirect_out::err_();
#endif
}



typedef void (*F_FLUSHING_FUNC)(ostream&);

void                Endl( ostream& );

void                Flush( ostream& );


}   // namespace csv



inline csv::ostream &
operator<<( csv::ostream &          io_rStream,
            csv::F_FLUSHING_FUNC    i_fFlushingFunc )
{
#ifndef CSV_NO_IOSTREAMS
//    (*i_fFlushingFunc)( io_rStream, csv::redirect_out::useme_(), 0 );
    (*i_fFlushingFunc)( io_rStream );
#else
    csv_assert( csv::redirect_out::useme_() );
    (*i_fFlushingFunc)( io_rStream, true, 0 );
#endif
    return io_rStream;
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
