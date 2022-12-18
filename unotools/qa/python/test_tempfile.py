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


class TestTempFile(unittest.TestCase):
    """Test temporary file object created from com.sun.star.io.TempFile"""

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def setUp(self):
        self.file_data = uno.ByteSequence(b"some data")

        service_manager = self._uno.getContext().getServiceManager()
        if service_manager is None:
            raise RuntimeError("Cannot create service factory!")

        try:
            self.file_access = service_manager.createInstance("com.sun.star.ucb.SimpleFileAccess")
            if self.file_access is None:
                raise RuntimeError("Cannot get simple access!")
        except Exception as e:
            raise RuntimeError(f"Cannot get simple file access! {e}")

        self.temp_file = service_manager.createInstance("com.sun.star.io.TempFile")
        has_xtempfile_if = bool([
            1 for type_info in self.temp_file.getTypes()
            if type_info.typeName == "com.sun.star.io.XTempFile"
        ])
        if not has_xtempfile_if:
            raise RuntimeError("Cannot get XTempFile interface.")

    def close_temp_file(self) -> None:
        stream = self.temp_file.getOutputStream()
        if stream is None:
            raise RuntimeError("Cannot get output stream")
        stream.closeOutput()
        stream = self.temp_file.getInputStream()
        if stream is None:
            raise RuntimeError("Cannot get input stream")
        stream.closeInput()
        print("Tempfile closed successfully.")

    def read_bytes_with_stream(self) -> uno.ByteSequence:
        input_stream = self.temp_file.getInputStream()
        if input_stream is None:
            raise RuntimeError("Cannot get input stream from tempfile.")
        nbytes, read_data = input_stream.readBytes(None, len(self.file_data))
        print("Read", nbytes, "bytes from tempfile successfully.")
        return read_data

    def read_directly_from_temp_file(self, file_url: str) -> uno.ByteSequence:
        print("Attempting to read directly from", file_url)
        input_stream = self.file_access.openFileRead(file_url)
        if input_stream is None:
            raise RuntimeError("Cannot create input stream from URL.")
        nbytes, read_data = input_stream.readBytes(None, len(self.file_data))
        print("Read", nbytes, "bytes directly from tempfile successfully.")
        return read_data

    def write_bytes_with_stream(self) -> None:
        output_stream = self.temp_file.getOutputStream()
        if output_stream is None:
            raise RuntimeError("Cannot get output stream.")
        output_stream.writeBytes(self.file_data)
        output_stream.flush()
        print("Write", len(self.file_data), "bytes to tempfile successfully.")

    def get_temp_file_url(self) -> str:
        uri = self.temp_file.Uri
        if not uri:
            raise RuntimeError("Temporary file not valid.")
        return uri

    def get_temp_file_name(self) -> str:
        file_name = self.temp_file.ResourceName
        if not file_name:
            raise RuntimeError("Temporary file not valid.")
        return file_name

    def test_01(self):
        file_uri = self.get_temp_file_url()
        file_name = self.get_temp_file_name()
        print("Tempfile URL:", file_uri)
        print("Tempfile name:", file_name)
        self.assertTrue(
            file_uri.endswith(file_name.replace("\\", "/")),
            "FILE NAME AND URL DO NOT MATCH.",
        )

        # write to the stream using the service.
        self.write_bytes_with_stream()

        # check the result by reading from the service.
        self.temp_file.seek(0)
        read_data = self.read_bytes_with_stream()
        self.assertEqual(self.file_data, read_data, "Tempfile outputs false data!")

        # check the result by reading from the file directly.
        read_data = self.read_directly_from_temp_file(file_uri)
        self.assertEqual(self.file_data, read_data, "Tempfile contains false data!")

        # close the object(by closing input and output), check that the file
        # was removed.
        self.temp_file.RemoveFile = False
        # After tempfile is closed, file name cannot be got from a TempFile object.
        file_name = self.temp_file.ResourceName
        self.close_temp_file()
        self.assertTrue(
            self.file_access.exists(file_name), "TempFile mistakenly removed.",
        )

        # Finally, cleanup this temp file.
        self.file_access.kill(file_name)

    def test_02(self):
        self.write_bytes_with_stream()
        file_url = self.get_temp_file_url()
        # let the service not to remove the URL.
        self.temp_file.RemoveFile = False
        # close the tempfile by closing input and output.
        self.close_temp_file()
        # check that the file is still available.
        read_data = self.read_directly_from_temp_file(file_url)
        self.assertEqual(self.file_data, read_data, "Tempfile contains false data!")


if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
