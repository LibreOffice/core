# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesnonfreemisc,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates-nonfree/misc/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesnonfreemisc,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesnonfreemisc,$(LIBO_SHARE_FOLDER)/template,misc,\
	cs/dummy_templates-nonfree.txt \
	de/dummy_templates-nonfree.txt \
	en-US/achievement_1.ott \
	en-US/athletic_achievement.ott \
	en-US/attendance_award.ott \
	en-US/award_of_merit.ott \
	en-US/best_suggestion.ott \
	en-US/bulletin_2.ott \
	en-US/bulletin_3.ott \
	en-US/event_calendar_1.ott \
	en-US/event_poster_1.ott \
	en-US/event_poster_2.ott \
	en-US/event_poster.ott \
	en-US/excellence.ott \
	en-US/first_place.ott \
	en-US/great_idea_award.ott \
	en-US/Landscape_Monthly_Calendar.ots \
	en-US/school_newsletter_1.ott \
	en-US/school_newsletter_2.ott \
	en-US/table_tent_landscape.ott \
	en-US/trifold_brochure.ott \
	es/dummy_templates-nonfree.txt \
	fi/dummy_templates-nonfree.txt \
	fr/dummy_templates-nonfree.txt \
	hu/Bulletin_2.ott \
	hu/Bulletin_3.ott \
	hu/Event_poster.ott \
	hu/Kituntetes.ott \
	it/dummy_templates-nonfree.txt \
	ja/dummy_templates-nonfree.txt \
	ka/dummy_templates-nonfree.txt \
	nl/dummy_templates-nonfree.txt \
	pl/dummy_templates-nonfree.txt \
	pt-BR/dummy_templates-nonfree.txt \
	sv/dummy_templates-nonfree.txt \
	tr/dummy_templates-nonfree.txt \
	zh-CN/dummy_templates-nonfree.txt \
))

# vim: set noet sw=4 ts=4:
