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

    void                re_endl()               { do_re_endl(); }
    void                re_flush()              { do_re_flush(); }

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
    virtual void        do_re_endl() = 0;
    virtual void        do_re_flush() = 0;

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



typedef void (*F_FLUSHING_FUNC)(ostream&, bool, int*);

void                Endl( ostream&, bool, int* );

void                Flush( ostream&, bool, int* );


}   // namespace csv



inline csv::ostream &
operator<<( csv::ostream &          io_rStream,
            csv::F_FLUSHING_FUNC    i_fFlushingFunc )
{
#ifndef CSV_NO_IOSTREAMS
//    (*i_fFlushingFunc)( io_rStream, csv::redirect_out::useme_(), 0 );
    (*i_fFlushingFunc)( io_rStream, false, 0 );
#else
    csv_assert( csv::redirect_out::useme_() );
    (*i_fFlushingFunc)( io_rStream, true, 0 );
#endif
    return io_rStream;
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
