#!/usr/bin/env bash

# probably obsoletes extract_translator.sh
# Historical list of translators was collected with the following command in browser/po:
# for i in *.po;do git log --follow -p $i | grep "Last-Translator" | sed -e "s/^.*: //" -e "s/<.*$//";done | sort -u
# Then after some cleanup (duplicated names etc.) we get the ORIG_LIST which will never change.
# Then we can get the delta after 2021-11-26 with git shortlog: NEW_LIST.
# Then we merge the two lists and print the result.

if [ $# -ne 1 ]
then
    echo "Usage: $0 <online-git-directory>"
    exit 1
fi

PWD=$(pwd)
cd $1

ORIG_LIST=$(mktemp)
cat <<EOF >$ORIG_LIST
김주현
Abdul Mukti Nurrohman
Abduqadir Abliz
abidin toumi
Adam Rak
Adolfo Jayme Barrientos
Ákos Nagy
Allan Nordhøy
Andika Triwidada
Andras Timar
Andreas Pettersson
Andrej Shadura
André Marcelo Alvarenga
Andrew Lee (李健秋)
Artem
Asier Sarasua Garmendia
Ayhan Yalçınsoy
bact
Baurzhan Muftakhidinov
belkacem77
Besnik Bleta
bormant
bruh
Budi Aryo
ButterflyOfFire
Carlos Moreira
Cédric Valmary
Cheng-Chia Tseng
Christian Kühl
Christian Lohmaier
Còdaze Veneto
Cor Nouws
DaeHyun Sung(성대현, 成大鉉)
David Lamhauge
Dimitris Spingos
doanmanhduy75
Donald Rogers
Eloy Crespo
Eugenia Russell
Felipe Viggiano
Flávio José de Siqueira Cavalc
Florian
FW
Gábor Kelemen
George Kitsoukakis
Giacomo Bertolotti
Gontzal Manuel Pujana Onaindia
Harri Pitkänen
HelaBasa
Hirae
Ihor Hordiichuk
Imanuel Ronaldo
Jan Holesovsky
Jean-Baptiste Faure
Jesper Hertel
J. Lavoie
Joachim Vincent
Joan Montané
Jörn Liebau
Karl Morten Ramberg
kees538
K. Herbert
koffevar
Kolbjørn Stuestøl
Kruno
Lars Kirschmann
Leif-Jöran Olsson
Leif Lodahl
liimee
LL Magical
Lukáš Jelínek
Luna Jernberg
Marco Cecchetti
Marco Marega
Marc Rodrigues
Martin Srebotnjak
Masa Murakami
Matthaiks
Mehmet Sait Gülmez
Michael Bauer
Michael Wolf
Michalis
Mihail Balabanov
Mike Kaganski
Milo Ivir
Miloš Šrámek
Ming Hua
Mirsad
Modestas Rimkus
Muhammet Kara
Muḥend Velqasem
naniud
Nathan
Naveen
Necdet Yucel
Nguyen Trung Kien
Nguyen Tuan Anh
Niklas Johansson
No Ne
Oğuz Ersen
Olav Dahlum
Olexandr Pylypchuk
Olivier Hallot
Osoitz
Oymate
pan93412
Paul Roos
Pedro Pinto Silva
phlostically
Piotr Rudzki
Pranav Kant
Quentin Pagès
raal
Reza Almanda
Rhoslyn Prys
Rizal Muttaqin
Rob Pearson
Roman Vasylyshyn
Ron Stephen Mathew
Rybnicek-PCHELP
Saikeo
Samson B
SC
Sebastiaan Veld
Sérgio Marques
Sérgio Morais
Slimane Selyan Amiri
So
Sophie Gautier
ssantos
Stanislav Horáček
Steen Rønnow
Stratos Kostidis
Stuart Swales
Sveinn í Felli
Szymon Kłos
Thais Vieira
Tor Lillqvist
Tymofii Lytvynenko
uzadmin
Valter Mura
VenetoABC
vpanter
wck317
William Gathoye
wxf26054
Xosé
Yaron Shahrabani
Zhou Nan
Андрій Бандура
Євген Кондратюк
امير محمد العمري
غادة الذياب
வே. இளஞ்செழியன் (Ve. Elanjelian)
日陰のコスモス
村上正記
琨珑 锁
EOF

NEW_LIST=$(mktemp)

git shortlog -n -s \
    browser/po \
    android/app/src/main/res/values-* \
    android/lib/src/main/res/values-* \
    ios/Mobile/Resources/Settings.bundle \
| awk -F '\t' '{print $2}' \
| grep -v Weblate \
| grep -v transifex-integration \
> $NEW_LIST

sort -u $ORIG_LIST $NEW_LIST | sed -z 's/\n/; /g;s/; $/\n/'
cd $PWD
rm $ORIG_LIST $NEW_LIST
