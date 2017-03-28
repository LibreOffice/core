# Pravila za automatsko ispravljanje, iznimke kraja rečenice te iznimke ispravljanja dvaju velikih početnih slova na početku riječi

Pravila za automatsko ispravljanje nisu dio ni računalne provjere pravopisa ni računalne provjere gramatike, ali pomažu prilagoditi neke automatizirane LibreOfficeove mehanizme ispravljanja teksta jeziku na kojemu se piše.

LibreOffice iza točke automatski ispravlja malo slovo u veliko jer ono što slijedi iza točke tretira kao početak nove rečenice. Ipak, nije svaka točka oznaka kraja rečenice, iznimke mogu biti kratice koje se pišu s točkom (kratica 'npr.'), redni brojevi i drugo. (Datoteka SentenceExceptList.xml)

Automatsko će ispravljanje zamijeniti i dva velika slova na početku riječi pa će MOzilla biti ispravljena u Mozilla. Međutim, postoje situacije kada je takvo isptavljanje nepoželjeno: MHz, MWh. (Datoteka WordExceptList.xml)

Treća je važna datoteka DocumentList.xml u kojoj su definirana pravila zamjene jedne sekvencije znakova drugom sekvencijom. Tako će :_2: dati ₂ (zamjena H:_2:O s H₂O), a :autorska prava: će dati znak ©.

---

Službena dokumentacija:

https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras

---

Sadržaj ovoga repozitorija

* **umetanje-automatskih-ispravaka.md** &mdash; upute kako dodati pravila na računalo za korištenje uz LibreOffice
* **dokumentacija.md** &mdash; opisane važne datoteke, dokumentirane odluke i potencijalni problemi
* **dodano-na-documentlist.txt** &mdash; popis unosa koji su dodani u datoteku DocumentList.xml mimo osnovnih/inicijalnih
* **DocumentList.xml** &mdash; pravila za automatsko ispravljanje
* **SentenceExceptList.xml** &mdash; popis kratica s točkom koje zapravo ne označavaju kraj rečenice
* **WordExceptList.xml** &mdash; popis riječi i kratica koje se pišu s dva velika početna slova
* **licence.md** &mdash; licencija i informacije za kontakt

---

Repozitorij i kontakt: https://github.com/krunose/libo-acorr-hr

---

Inačica 2017-03-27
