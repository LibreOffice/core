= hunspell-id
:toc:
:sectanchors:
:sectlinks:

Proyek ini adalah kelanjutan dari ekstensi Kamus Indonesia pada
https://extensions.libreoffice.org/extension-center/indonesian-dictionary-kamus-indonesia-by-benitius/releases/2.0[LibreOffice^].

== Unduh

Unduh arsip zip yang terbaru lewat tautan berikut
https://github.com/shuLhan/hunspell-id/archive/master.zip[master.zip^],
atau salin dengan `git`

----
$ git clone git@github.com:shuLhan/hunspell-id.git
----

Jika mengunduh arsip zip, ekstrak berkas zip tersebut.


== Pasang

=== Sistem Operasi GNU/Linux atau BSD

Langkah pemasangan secara manual,

	$ sudo make install

Perintah tersebut juga mengikutkan pemasangan berkas eja untuk program myspell
dan vim.

Untuk menghapus kembali pemasangan, lakukan

	$ sudo make uninstall

Paket untuk GNU/Linux tersedia sebagai berikut.

Arch Linux:: Pengguna Arch dapat memasang paket ini lewat
git submodule dalam direktori AUR atau
https://aur.archlinux.org/packages/hunspell-id-git/[AUR^].

Debian (dan turunan):: Lihat dalam direktori debian.

Solus:: Paket ini dapat dipasang dengan cara,
+
--
----
$ sudo eopkg it hunspell-id
----
--

=== Ekstensi LibreOffice

Pindah ke direktori sumber repositori,

----
$ cd hunspell-id
----

Eksekusi skrip `build.sh` pada direktori sumber

----
$ ./build.sh
----

Skrip tersebut akan menghasilkan berkas `hunspell-id.oxt`.
Buka pengatur berkas, dan klik dua kali pada berkas `hunspell-id.oxt`, nanti
layanan pemasangan ekstensi LibreOffice akan muncul.

=== Vim

Buat direktori `$HOME/.vim/spell`, kemudian salin atau buat _symlink_ dari
berkas `vim/id.utf-8.spl` ke dalam direktori tersebut.


== Sejarah

Versi 1.0, 1.1, dan 1.2 diciptakan tahun 2004 dengan bantuan Kurniadi dan
Volker Mueller serta Arno Brevoort, yang menyumbangkan daftar kata mereka
sebagai dasar penyusunan berkas tersebut bagi program MySpell, yang dipakai
oleh OpenOffice.org.
Sesudah itu bertahun-tahun lamanya tidak dikerjakan lagi.

Tanggal 05 Mei 2009 Ammar Shadiq menuangkan berkas-berkas lama itu dalam
bentuk ekstensi bagi OpenOffice.
Ekstensi yang sama ini diunggah ke LibreOffice pada tanggal 19 Mei 2012
sebagai Indonesian dictionary - Kamus Indonesia 1.0.
Viko Adi Rahmawan kemudian menguji ekstensi ini kembali dan pada tanggal 16
Mei 2014 diterbitkan sebagai Indonesian dictionary - Kamus Indonesia 1.1 bagi
LibreOffice 4.0.

Versi 2.0 merupakan pengerjaan ulang dari kedua berkas `id_ID.dic` dan
`id_ID.aff` agar lebih sesuai dengan sifat Bahasa Indonesia.
Berkas .aff (affiks) diperbaharui sehingga jauh lebih sesuai
dengan sifat bahasa Indonesia dengan awalan, akhiran dan imbuhan.
Berkas .dic (daftar kata) diperiksa dengan semua lemma dari Kamus Besar Bahasa
Indonesia, edisi 3.
Berkas hyphenator tidak mengalami perubahan.

== Lisensi

----
Copyright (C) 2004-2022 hunspell-id Authors

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version, with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
A copy of the GNU Lesser General Public License is in the file COPYING.
----

Daftar pengembang yang diketahui (diurut berdasarkan abjad),

* Ammar Shadiq
* Andika Triwidada (Untuk paket Debian)
* Arno Brevoort
* Benitius Brevoort
* Kurniadi
* Viko Adi Rahmawan
* Volker Mueller
* Yurizal Susanto (Untuk paket Solus)

Jika Anda pernah berkontribusi sebelumnya tapi tidak tercantum di sini
silahkan kirim email ke ms@kilabit.info.

// SPDX-FileCopyrightText: 2015 hunspell-id Authors
// SPDX-License-Identifier: LGPL-3.0-only
