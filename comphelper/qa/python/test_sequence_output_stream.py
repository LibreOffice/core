#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import unittest
import uno

from org.libreoffice.unotest import UnoInProcess


class TestSequenceOutputStream(unittest.TestCase):
    """Test com.sun.star.io.SequenceOutputStream"""

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def setUp(self):
        self.data = uno.ByteSequence(b"some data")
        try:
            self.service_manager = self._uno.getContext().getServiceManager()
        except Exception:
            raise RuntimeError("Cannot create service factory!")
        if self.service_manager is None:
            raise RuntimeError("Cannot create service factory!")

    def test_stream(self):
        try:
            seq_output_stream = self.service_manager.createInstance(
                "com.sun.star.io.SequenceOutputStream"
            )
            seq_output_stream.writeBytes(self.data)

            # Append the same content once again
            seq_output_stream.writeBytes(self.data)

            written_bytes = seq_output_stream.getWrittenBytes()

            self.assertEqual(
                len(self.data) * 2,
                len(written_bytes),
                "SequenceOutputStream::getWrittenBytes() - wrong amount of bytes returned",
            )

            # create SequenceInputstream
            seq_input_stream = self.service_manager.createInstanceWithArguments(
                "com.sun.star.io.SequenceInputStream", (written_bytes,)
            )

            # read from the stream
            nbytes_read, read_bytes = seq_input_stream.readBytes(None, len(self.data) * 2 + 1)
            self.assertEqual(
                len(self.data) * 2,
                nbytes_read,
                "SequenceInputStream::readBytes() - "
                f"wrong amount of bytes returned {len(self.data) * 2} vs {nbytes_read}",
            )

            # close the streams
            seq_output_stream.closeOutput()
            seq_input_stream.closeInput()

            expected = uno.ByteSequence(self.data.value * 2)
            self.assertEqual(expected, written_bytes, "Written array not identical to original.")
            self.assertEqual(expected, read_bytes, "Read array not identical to original.")
        except Exception as e:
            self.fail(f"Exception: {e}")


if __name__ == "__main__":
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
