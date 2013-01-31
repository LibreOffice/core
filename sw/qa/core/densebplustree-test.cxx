/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <densebplustree.hxx>
#include <bparr.hxx>

#include <sys/time.h>

class BigPtrEntryMock : public BigPtrEntry
{
public:
    BigPtrEntryMock(sal_uLong count) : count_(count)
    {
    }

    ~BigPtrEntryMock()
    {
    }

    sal_uLong getCount() const
    {
        return count_;
    }

    void setCount(sal_uLong newCount)
    {
        count_ = newCount;
    }

    sal_uLong Position() const
    {
        return GetPos();
    }

private:
    sal_uLong count_;
};

void print_time( const char* msg, const struct timeval &before, const struct timeval &after )
{
    time_t sec = after.tv_sec - before.tv_sec;
    suseconds_t usec = sec * 1000000 + after.tv_usec - before.tv_usec;

    printf( "%s: %ld msec\n", msg, usec / 1000 );
}

int main( int, char** )
{
#if 1
    struct timeval tv_before, tv_after;

    gettimeofday( &tv_before, NULL );
    BigPtrArray bparr;
    for ( int i = 0; i < 1000000; i++ )
        bparr.Insert( new BigPtrEntryMock(i), bparr.Count() );
    gettimeofday( &tv_after, NULL );
    print_time( "BigPtrArray - append", tv_before, tv_after );

    gettimeofday( &tv_before, NULL );
    BigPtrArray bparr2;
    for ( int i = 0; i < 1000000; i++ )
        bparr2.Insert( new BigPtrEntryMock(i), 0 );
    gettimeofday( &tv_after, NULL );
    print_time( "BigPtrArray - insert at front", tv_before, tv_after );

    gettimeofday( &tv_before, NULL );
    BigPtrArray bparr3;
    for ( int i = 0; i < 1000000; i++ )
        bparr3.Insert( new BigPtrEntryMock(i), bparr3.Count() / 2 );
    gettimeofday( &tv_after, NULL );
    print_time( "BigPtrArray - insert in the middle", tv_before, tv_after );

    gettimeofday( &tv_before, NULL );
    DenseBPlusTree< int, BigPtrEntryMock* > aTest;
    for ( int i = 0; i < 1000000; ++i )
        aTest.Insert( new BigPtrEntryMock(i), aTest.Count() );
    gettimeofday( &tv_after, NULL );
    print_time( "DenseBPlusTree - append", tv_before, tv_after );

    gettimeofday( &tv_before, NULL );
    DenseBPlusTree< int, BigPtrEntryMock* > aTest2;
    for ( int i = 0; i < 1000000; ++i )
        aTest2.Insert( new BigPtrEntryMock(i), 0 );
    gettimeofday( &tv_after, NULL );
    print_time( "DenseBPlusTree - insert at front", tv_before, tv_after );

    gettimeofday( &tv_before, NULL );
    DenseBPlusTree< int, BigPtrEntryMock* > aTest3;
    for ( int i = 0; i < 1000000; ++i )
        aTest3.Insert( new BigPtrEntryMock(i), aTest3.Count() / 2 );
    gettimeofday( &tv_after, NULL );
    print_time( "DenseBPlusTree - insert in the middle", tv_before, tv_after );
#endif

#if 0
    DenseBPlusTree< int, int > aNumbers;
    aNumbers.Insert( 20, 0 );
    aNumbers.Insert( 10, 0 );
    aNumbers.Insert( 30, 2 );
    aNumbers.Insert( 1000, 3 );
    for ( int i = 0; i < 100; ++i )
        aNumbers.Insert( i, 3 );//aNumbers.Count() );
    aNumbers.dump();
#endif

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
