# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,bluez_bluetooth_inc,$(SRCDIR)/bluez_bluetooth/inc))

$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/bluetooth.h,bluetooth/bluetooth.h))
$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/hci.h,bluetooth/hci.h))
$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/hci_lib.h,bluetooth/hci_lib.h))
$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/l2cap.h,bluetooth/l2cap.h))
$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/rfcomm.h,bluetooth/rfcomm.h))
$(eval $(call gb_Package_add_file,bluez_bluetooth_inc,inc/bluetooth/sco.h,bluetooth/sco.h))

# vim: set noet sw=4 ts=4:
