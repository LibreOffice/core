/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
//
//	stdafx.h : include file for standard system include files,
//	or project specific include files that are used frequently,
//	but are changed infrequently
//
#pragma once

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <tchar.h>
#include <time.h>

#define INITGUIDS
#include <initguid.h>

#include <cesync.h>
#include <replfilt.h>

#include "XMergeSync.h"

//
//	This declares the one & only instance of the CXMergeSyncModule class.
//	You can access any public members of this class through the
//	global _Module.  (Its definition is in XMergeSync.cpp.)
//
extern CXMergeSyncModule _Module;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
