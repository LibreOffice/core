/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// ZipException.h: interface for the ZipException class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ZIPEXCPTN_HXX_INCLUDED
#define ZIPEXCPTN_HXX_INCLUDED

#include <stdexcept>

//------------------------------------------
/**
*/
class RuntimeException : public std::exception
{
public:
    RuntimeException(int Error);
    virtual ~RuntimeException() throw();

    int GetErrorCode() const;

private:
    int m_Error;
};

//------------------------------------------
/**
*/
class ZipException : public RuntimeException
{
public:
    ZipException(int Error);

    virtual const char* what() const throw();
};

//------------------------------------------
/**
*/
class Win32Exception : public RuntimeException
{
public:
    Win32Exception(int Error);
    virtual ~Win32Exception() throw();

    virtual const char* what() const throw();

private:
    void* m_MsgBuff;
};

//------------------------------------------
/**
*/
class ZipContentMissException : public ZipException
{
public:
    ZipContentMissException(int Error);
};

//------------------------------------------
/**
*/
class AccessViolationException : public Win32Exception
{
public:
    AccessViolationException(int Error);
};

//------------------------------------------
/**
*/
class IOException : public Win32Exception
{
public:
    IOException(int Error);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
