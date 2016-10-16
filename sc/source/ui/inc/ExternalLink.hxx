/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_INC_EXTERNALDATAIMPORT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_EXTERNALDATAIMPORT_HXX

#include <datastream.hxx>

namespace sc {

enum class SourceAvailability
{
	OFFLINE = 0,
	ONLINE
};

enum class SourceType
{
	CSV = 0,
	JSON,
	XML,
	HTML,
	CALC,
	EXCEL,
	SQL,
	BASE,
	ACCESS,
};

class ExternalLink						// TODO : Link ScDbData to this. Maybe use callbacks to update Database?
{
	OUString mURL;
	SourceAvailability mAvailability;
	SourceType mType;

	double mRefreshRate;				// Replace double with something more platform independent ?

public:
	ExternalLink(OUString& rUrl):
		mURL(rUrl)
		{}
	~ExternalLink();

	void StartImport();
	void StopImport();
	void Refresh();

	void SetRefreshRate(double mRefreshRate);

private:
	void DecodeURL();			// Decodes the URL and sets mAvailability and mType variables

};

}
#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
