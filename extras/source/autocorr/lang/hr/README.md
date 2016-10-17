# Pravila za automatsko ispravljanje, iznimke kraja rečenice te iznimke u pisanju velikog i malog slova iz grafostilističkih razloga
U ovom se dokumentu ukratko opisuje LibreOfficeova značajka automatskog ispravljanja. Za ovu su značajku važne tri datoteke: DocumentList.xml, SentenceExceptList.xml te datoteka WordExceptList.xml. Svaka se od njih ukratko opisuje u nastavku dokumenta uz kratak komentar.

Želite li koristiti ovaj popis sa starijom inačicom LibreOfficea (koja ne sadrži sva pravila s ovog popisa), pogledajte upute iz datoteke '[umetanje-automatskih-ispravaka.md](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/umetanje-automatskih-ispravaka.md)'.

Službeni se opisi pojedinih datoteka nalaze na

[https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras](https://wiki.documentfoundation.org/LibreOffice_Localization_Guide/Advanced_Source_Code_Modifications#Extras)


## DocumentList.xml
Automatsko ispravljanje riječi. Zamjena određenih kombinacija znakova s novim znakom. Recimo X:^2: će dati X². [[Mogući ispravci](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/DocumentList.xml)]

U ovu je datoteku poželjno dodavati i one ispravke koji su **nedvojbeno** zatipci i nije vjerojatno da su pogreške koje bi netko mogao prezentirati u dokumentu prilikom učenja jezika.

Znači, u hrvatskom se kada ispred dolazi nenaglašeni oblik pomoćnog glagola biti pišu krnji infinitivi: 'tražit ću', ne 'tražiti ću'. Nije dobro u ovu datoteku unositi ispravke 'tražiti ću' > 'tražit ću' jer netko može pisati nešto o hrvatskom pravopisu, gramatici ili pravogovoru i možda namjerno želi napisati 'tražiti ću' kao primjer česte pogreške. U tom slučaju autor **želi** 'tražiti ću' i ne želi da LibreOffice to automatski ispravlja bez ikakva upozorenja i podcrtavanja.

Zadaća je računalne provjere pravopisa i gramatike da takve situacije detektiraju i upozore korisnika (podcrtaju) te ostave na korisniku da odluči što napraviti, a ne da se ispravci vrše bez upozorenja kada postoji mogućnost da i nisu željeni!

U ovu datoteku **treba** dodati ono što je nedvojbeno zatipak i što bi nepotrebno opterećivalo rječnik ili provjeru pravopisa, odnosno što bi povećavalo vrijeme provjere napisanog u vidu odobravanja ili ispravljanja onoga što se podcrtalo provjerom pravopisa ili gramatike. Primjere poput 'pokuša tću' treba dodati na ovaj popis kako bi LibreOffice to automatski ispravio u 'pokušat ću' jer nema potrebe da tu korisnik gubi vrijeme na ispravljanje podcrtanog kada je jasno što je trebalo pisati i to treba ispraviti odmah i automatski.

Naravno, navedeni bi se primjer mogao ispraviti i pravilima za provjeru gramatike, ali u tom bi slučaju korisnik ručno morao odobriti ili ispraviti svaku takvu grešku. U ovom slučaju za tim nema potrebe, zašto pogreške takva tipa LibreOffice ne bi ispravljao sam?

Ipak, treba biti vrlo oprezan. U engleskoj se inačici pravila za utomatsko ispravljanje nalazi pravilo koje zatipak 'nwe' zamjenjuje sa 'new', što je se na prvi pogled čini dobrim rješenjem. Međutim, jedan je korisnik automatskog ispravljanja za engleski jezik trebao i **želio** napisati 'nwe' jer je to, naime, kratica (oznaka) za jedan mali jezik u Kamerunu. LibreOffice je svaki puta u dokumentu 'nwe' zamijenio s 'new' bez ikakvog upozorenja i korisnik je primijetio grešku kada je već bilo kasno. U ovom bi slučaju podcrtavanje pomoću provjere pravopisa i/ili gramatike bilo bolje rješnje jer bi korisnik bio upozoren na potencijalnu pogrešku, ali sâm sadržaj ne bi bio uništen neželjenim ispravljanjem.

---

U ovoj se datoteci nalaze zapravo dvije grupe automatskih ispravaka.

Jednom se grupom mogu smatrati unosi odnosno tekst koji je zapravo kratica za jednostavniji unos složenijih izraza. Tako je :^2: zapravo kratica unutar automatskog ispravljanja za ². Ova grupa obično počinje i završava s dvotočkom i korisnik mora naučiti te kratice i koristi ih **svjesno** i **namjerno**.

Problematičnija je druga grupa ispravaka koja obuhvaća primjere poput 'nwe' ili 'pokuša tću'. Problematičnija zbog toga što korisnik takve izraze ne koristi kao prečicu za drugi, složeniji izraz nego LibreOffice pokreće takve ispravke bez korisnikova znanja. Korisnik ne može znati što se sve nalazi na popisu za automatsko ispravljanje, niti onaj tko unose radi može predvidjeti svaku moguću uporabu određenog slijeda znakova, a LibreOffice će izvršiti automatsko ispravljanje u situaciji koji nisu predvidjeli niti korisnik niti autor pravila. Iz tog je razloga važno biti vrlo oprezan oko onga što se dodaje na popis u kontekstu ove grupe automatskih ispravaka.

---

Popis se zamjena koje su dodani mimo originalnih nalazi u datoteci '[dodano.txt](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/blob/master/dodano-na-documentlist.txt)'. Iako je 'originalno' relativan pojam, naime ne postoji standardni set ispravaka jer izgleda da svaki jezik ima svoje ispravke i međusobno su potpuno neovisni. Tako ih neki jezici imaju i nekoliko tisuća, a neki desetak. Ovdje se 'originalno' odnosi na zatečeno stanje.

Brojni su ispravci iz ove datoteke uklonjeni jer su smatrani popotrebnima prema kriterijima iznesenima u ovoj datoteci. Za ispravak se tih obrisanih primjera brine rječnik za provjeru pravopisa.


## SentenceExceptList.xml
Točka označava kraj rečenice. LibreOffice automatski riječ iza točke – početak nove rečenice – piše velikim slovom. Ipak, nije svaka točka i znak za kraj rečenice.

Iza nekih se skraćenica i kratica piše točka. Takva je kratica dio rečenice a ne njezin kraj. Da bi se spriječilo automatsko ispravljanje malog slova u veliko, kratice koje se nalaze na ovom popisu označavaju riječi s točkom koje se obično ne nalaze na kraju rečenice i sprječavaju LibreOffice da otežava pisanje nepotrebnim umetanjem velikog slova.

Jedna je od takvih skraćenica i 'npr.'. Kada je ne bi bilo na popisu, LibreOffice bi bi rečenicu 'Željeli smo napraviti nešto važno, npr. rječnik.' automatski ispravio u '~ npr. Rječnik.'

Znači, na ovom se popisu nalaze riječi koje LibreOffice inače (pogrešno) interpretira kao kraj rečenice.

Treba biti oprezan i ne uključivati kratice koje su izrazom jednake riječima koje doista i mogu stajati na kraju rečenice: 'čest.' od 'čestica' i 'čest' kao pridjev koji može stajati na kraju rečenice pa onda iza njego može stajati točka i sljedeća se riječ piše velikim slovom.

Iza rednih brojeva LibreOffice ne ispravlja mala slova.

### Kriterij za određivanje što uključiti
Nije jednostavno odrediti čvrste kriterije po kojima bi se jednostavno moglo odlučiti koju kraticu uključiti a koju ne. Neke se kratice mogu i ne moraju nalaziti na kraju rečenice. Neke su kratice (metor. od meteorološki ili med. od medicinski) izrazom jednake drugim rječima (med, meteor) i tu treba biti oprezan.

Ne preostaje drugo nego se voditi vlastitim jezičnim osjećajem i intuicijom. Vjerojatnije je da će se kratica meteor. javiti češće u različitim tipovima tekstova i diskurza nego riječ 'meteor' koji je uglavnom ograničena na jednu struku i jedan (uži) kontekst. Vjerojatnost da se radi o riječi 'meteor' dodatno se smanjuje ako se u obzir uzme da do zabune dolazi samo ako se riječ 'meteor' nađe na kraju rečenice te iza nje slijedi točka (ako nema točke, nema ni zabune). Iz tog je razloga uključena u popis.

Slično je i s kraticom med. od medicinski, medicina i sl.

**Valjalo bi u korpusu hrvatskog jezika stvarno i provjeriti što se i kada se javlja češće i prema tome korigirati popis.**

Na kraju se ove datoteke nalaze kratice koje se jednako vjerojatno (prema osobnom jezičnom osjećaju i iskustvu) mogu jednako nalaziti u sredini rečenice i na kraju pa nisu uključene u popis kako bi se izbjegle greške. Taj bi se popis dodatno moga korigirati alatom za provjeru gramatike (LightProof, LanguageTool i drugo).

## WordExceptList.xml
LibreOffice automatski ispravlja pogreške poput 'rAčunalo' (> 'Računalo') i 'RAčunalo' (> 'Računalo').

Postoje riječi (kratice) koje se i pišu nešto drugačije. Najbolji je primjer OOo za 'OpenOffice.org'. Kada 'OOo' ne bi bio na ovom popisu, LibreOffice bi 'OOo' ispravio u 'Ooo' jer pretpostavlja da se samo prva riječ u riječi piše velikim slovom.

Valja primjetiti da one riječi koje se nalaze u Hunspellovu rječniku, LibreOffice neće automatski ispravljati. Tako se 'kHz' nalazi Hunspellovu rječniku za provjeru pravopisa i LibreOffice ovdje ne vrši automatski ispravak u 'Khz' iako se ova mjerna jednica (13. 10. 2016.) **ne nalazi** u datoteci WordExceptList.xml.

Takve bi primjere bilo uputnije uvrštavati u WordExceptList.xml datoteku negoli u rječnik za provjeru pravopisa jer takve riječi (OOo) nisu česte i u rječniku bi za provjeru pravopisa mogle biti nepoželjne: pojavljivale bi se kao prijedlozi, a najčešće bi bile pogrešno predložene zbog sličnosti s drugim riječima iz rječnika.


## dodano-na-documentlist.txt
U datoteci se navode unosi kojih nema na izvornom LibreOfficeovom popisu. Dodani su naknadno što je bilo potrebno radi lakšeg ispravljanja eventualnih pogrešaka i vođenja evidencije o tome što je točno naknadno uključeno na popis sa svakom novom inačicom.


## Umetanje-automatskih-ispravaka.md
U datoteci se opisuje postupak unošenja ovog popisa za automatsko ispravljanje u računalo.

Često su pravila za automatsko ispravljanje iz [ovog repozitorija](https://github.com/Kruno-S/acor_hr-HR-LibreOffice/) novija od onih koja se nalaze u izvornom kodu LibreOffice. Novi su ispravci dostupni tek u sljedećoj inačici LibreOfficea (napravi li se ažuriranje u vrijeme kada je aktualna inačica 5.1.5, nova će pravila biti dostupna tek u inačici 5.2.0).

Želi li netko dulje vrijeme koristiti (završnu) stabilnu inačicu, recimo 5.1.5 i ne želi koristiti početnu inačicu sljedeće grane, recimo 5.2.0, možda će željeti uključiti nova pravila za automatsko ispravljanje i u stariju, stabilnu inačicu koji koristi. Upute se mogu pronaći u spomenutoj datoteci.
